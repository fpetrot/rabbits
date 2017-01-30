# This file is part of Rabbits
# Copyright (C) 2015  Clement Deschamps and Luc Michel
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

require 'psych'
require 'optparse'

PARAM_TPL=(' ' * 8) + 'add_param("%{name}", Parameter<%{type}>("%{description}", %{default}, %{advanced}));'

DISCOVER_TPL=(' ' * 4) +
   'virtual void discover(const std::string &name, const PlatformDescription &params) {
        Parameters cp = get_params();
        cp.fill_from_description(params);

        %{class}::discover(name, cp, get_config());
    }
'


FACTORY_HEADER_TPL='
#include <rabbits/%{kind}/factory.h>
%{include}

namespace autogen {
namespace %{kind} {

class %{factory_class} : public %{Kind}Factory<%{class}> {
public:
    %{factory_class}(ConfigManager &config)
        : %{Kind}Factory<%{class}>(config, "%{type}", "%{description}" %{extra_ctor_params}) {
%{parameters}
    }

    virtual ~%{factory_class}() {}

%{discover}

};

}; /* namespace %{kind} */
}; /* namespace autogen */
'

INST_TPL=(' ' * 8) + 'c.get_%{kind}_manager().register_factory(std::make_shared<autogen::%{kind}::%{factory_class}>(c));
'

STATIC_LOADER_TPL='
#include <memory>

#include "rabbits/config/static_loader.h"
#include "rabbits/config/manager.h"

%{includes}

void StaticLoader::load(ConfigManager &c)
{
%{create_insts}
}
'

DYNAMIC_LOADER_TPL='
#include <vector>

#include <rabbits/config.h>
#include <rabbits/config/manager.h>
#include <rabbits/component/factory.h>
#include <rabbits/plugin/factory.h>
#include <rabbits/dynloader/dynloader.h>

%{includes}

namespace %{module_name}_autogen {

class RabbitsDynLoader {
public:
    static void load(ConfigManager &c) {
%{create_insts}
    }
};

};

extern "C" {
static const RabbitsDynamicInfo %{module_name}_dyn_info = {
  .name = "%{module_name}",
  .version_str = "%{module_version}"
};

int rabbits_dynamic_api_version(void)
{
    return RABBITS_API_VERSION;
}

const RabbitsDynamicInfo * rabbits_dynamic_info(void)
{
    return &%{module_name}_dyn_info;
}

void rabbits_dynamic_load(ConfigManager &config)
{
    %{module_name}_autogen::RabbitsDynLoader::load(config);
}

void rabbits_dynamic_unload(void)
{
}
}
'



def cc_ify(str)
  str.gsub(/-/, '_')
end

class DescrType
  @@descr_types = {}

  attr_reader :cc_type
  def initialize(type_name)
    @name = type_name
    @@descr_types[type_name] = self
  end

  def DescrType.get(type_name)
    if type_name.start_with?('vector(') then
      sub_type = type_name[/vector\((.*)\)/,1]
      sub_converter = get(sub_type)

      return nil unless sub_converter
      return DescrTypeVector.new('vector', sub_converter)
    end

    return @@descr_types[type_name]
  end
end

class DescrTypeString < DescrType
  def initialize(type_name)
    super(type_name)
    @cc_type = "std::string"
  end

  def convert(str)
    '"' + str.to_s + '"'
  end
end

def unit2factor(unit)
    case unit
    when 'k', 'K'
      1024
    when 'm', 'M'
      1024 * 1024
    when 'g', 'G'
      1024 * 1024 * 1024
    else
      1
    end
end

class DescrTypeInt < DescrType
  def initialize(type_name)
    super(type_name)

    if /u?int(8|16|32|64)/.match(type_name)
      @cc_type = type_name + '_t'
    else
      @cc_type = 'int'
    end
  end

  def convert(str)
    return str if str.is_a?(Integer)

    unit = str[-1]
    factor = unit2factor(unit)

    return str.to_i * factor
  end
end

class DescrTypeDouble < DescrType
  def initialize(type_name)
    super(type_name)
    @cc_type = 'double'
  end

  def convert(str)
    return str if str.is_a?(Float)

    unit = str[-1]
    factor = unit2factor(unit)

    return str.to_f * factor
  end
end

class DescrTypeBoolean < DescrType
  def initialize(type_name)
    super(type_name)
    @cc_type = 'bool'
  end

  def convert(str)
    return str if (str == false || str == true)

    case str.to_s.downcase
    when 'true', '1'
      return true
    when 'false', '0'
      return false
    end

    raise StandardError.new("Cannot convert `#{str}' to boolean")
  end
end

class DescrTypeTime < DescrType
  def initialize(type_name)
    super(type_name)
    @cc_type = 'sc_core::sc_time'
  end

  def convert(str)
    unit_idx = str.index(/[fpnum]s$/)

    unit = str[unit_idx..-1].upcase if unit_idx
    unit = 'PS' unless unit_idx

    unit_idx = -1 unless unit
    value = str[0..unit_idx].to_f

    return "sc_core::sc_time(#{value}, sc_core::SC_#{unit})"
  end
end

class DescrTypeVector < DescrType
  def initialize(type_name, sub_converter)
    super(type_name)
    @sub_converter = sub_converter
    @cc_type = 'std::vector< ' + sub_converter.cc_type + ' > '
  end

  def convert(vec)
    raise StandardError.new("#{vec} is not a vector") unless vec.is_a?(Array)

    ret = ""
    sep = "{ "

    vec.each do |e|
      ret += sep + @sub_converter.convert(e).to_s
      sep = ", "
    end

    if sep == "{ " then
      ret += sep
    end

    ret += " }"
    return ret
  end
end

DescrTypeString.new('string')
DescrTypeInt.new('integer')
DescrTypeInt.new('uint8')
DescrTypeInt.new('uint16')
DescrTypeInt.new('uint32')
DescrTypeInt.new('uint64')
DescrTypeInt.new('int8')
DescrTypeInt.new('int16')
DescrTypeInt.new('int32')
DescrTypeInt.new('int64')
DescrTypeDouble.new('double')
DescrTypeDouble.new('float')
DescrTypeBoolean.new('boolean')
DescrTypeTime.new('time')

class Parameter
  attr_accessor :name, :type, :default, :description, :advanced

  def initialize(name, descr)
    required = ['type', 'default', 'description']

    raise StandardError.new("Missing attribute(s): " + (required - descr.keys).join(",") + " for parameter `#{name}'") if (descr.keys & required).sort != required.sort

    converter = DescrType.get(descr['type'])

    raise StandardError.new("Unknown type `#{descr['type']}' for parameter `#{name}'") unless converter

    @name = name
    @type = converter.cc_type
    @default = converter.convert(descr['default'])
    @description = descr['description'].gsub("\n", '\n')
    @advanced = descr['advanced']
    @advanced = false unless @advanced
  end

  def get_print_args
    {
      :name => @name,
      :description => @description,
      :type => @type,
      :default => @default,
      :advanced => @advanced
    }
  end
end

class RabbitsModule
  attr_accessor :fn, :type, :description, :class, :include, :parameters

  def initialize(fn, descr, args)
    check_required(descr, ['type', 'description', 'class', 'include'])

    @args = args

    @fn = fn
    @type = descr['type']
    @class = descr['class']
    @include = descr['include']
    @description = descr['description'].gsub("\n", '\n')
    @parameters = {}

    descr['parameters'].each do |pname, pval|
      @parameters[pname] = Parameter.new(pname, pval)
    end if descr.key?('parameters')
  end

  def check_required(descr, req)
    raise StandardError.new("Missing attribute(s): " + (req - descr.keys).join(",")) if (descr.keys & req).sort != req.sort
  end

  def build_parameters
    @parameters.map do |_, p|
      PARAM_TPL % p.get_print_args
    end.join("\n")
  end

  def build_include
    dirname = File.dirname(@fn)

    if @include.is_a?(Array) then
      @include.map do |i|
        "#include \"#{dirname}/#{i}\""
      end.join("\n")
    else
      "#include \"#{dirname}/#{@include}\""
    end
  end

  def build_self_include
    '#include "' + @fn.gsub(@args.source_dir, @args.build_dir) + '.h"'
  end

  def get_print_args
    {
      :type => @type,
      :description => @description,
      :class => @class,
      :factory_class => cc_ify(@type) + "Factory",
      :include => build_include,
      :self_include => build_self_include,
      :parameters => build_parameters,
    }
  end

  def gen_factory
    FACTORY_HEADER_TPL % get_print_args
  end
end

class Component < RabbitsModule
  attr_accessor :implem, :discover, :prio

  def initialize(fn, descr, args)
    super(fn, descr, args)

    check_required(descr, ['implementation'])

    @implem = descr['implementation']
    @discover = descr['discover']
    @prio = descr['priority']
    @prio = 0 unless @prio
  end

  def build_discover
    if @discover then
      DISCOVER_TPL % { :class => @class }
    else
      ''
    end
  end

  def build_extra_ctor_params
    ', "' + @implem + '", ' + @prio.to_s()
  end

  def get_print_args
    super.merge({
      :extra_ctor_params => build_extra_ctor_params,
      :discover => build_discover,
      :kind => "component",
      :Kind => "Component",
    })
  end
end

class Plugin < RabbitsModule
  def initialize(fn, descr, args)
    super(fn, descr, args)
  end

  def get_print_args
    super.merge({
      :extra_ctor_params => '',
      :discover => '',
      :kind => "plugin",
      :Kind => "Plugin",
    })
  end

end

class Backend < RabbitsModule
  def initialize(fn, descr, args)
    super(fn, descr, args)
  end

  def build_discover
    if @discover then
      DISCOVER_TPL % { :class => @class }
    else
      ''
    end
  end

  def get_print_args
    super.merge({
      :extra_ctor_params => '',
      :discover => build_discover,
      :kind => "backend",
      :Kind => "Backend",
    })
  end
end

class StaticLoader
  def initialize(modules)
    @modules = modules
  end

  def generate
    includes = ""
    creates = ""

    @modules.each do |m|
      includes += m.build_self_include + "\n"
      creates += INST_TPL % m.get_print_args
    end

    STATIC_LOADER_TPL % { :includes => includes, :create_insts => creates }
  end
end


class LoaderGenerator
  def initialize(modules, tpl, args)
    @modules = modules
    @tpl = tpl
    @args = args
  end

  def generate
    includes = ""
    creates = ""

    @modules.each do |m|
      includes += m.build_self_include + "\n"
      creates += INST_TPL % m.get_print_args
    end

    @tpl % {
      :includes => includes,
      :create_insts => creates,
      :module_name => cc_ify(@args.modname),
      :module_version => @args.modver
    }
  end
end

class ParseArgs
  attr_accessor :mode, :out, :source_dir, :build_dir, :in_files, :modname, :modver

  def initialize
    out_file = nil
    @build_dir = ""
    @source_dir = ""
    @modname = ""
    @modver = ""

    OptionParser.new do |parser|
      parser.banner = "gen-factory [options] in.yml [...]"

      parser.separator("")
      parser.separator("Mode selection:")

      parser.on("--factory", "Factory generation mode") do
        @mode = :factory
      end

      parser.on("--static-loader", "Static loader generation mode") do
        @mode = :static
      end

      parser.on("--dyn-loader", "Dynamic loader generation mode") do
        @mode = :dynamic
      end

      parser.separator("")
      parser.separator("Static and dynamic loader mode options:")

      parser.on("-s SOURCE_DIR", "Source directory") do |dir|
        @source_dir = dir
      end

      parser.on("-b BUILD_DIR", "Build directory") do |dir|
        @build_dir = dir
      end

      parser.on("-m MODULE_NAME", "Module name") do |name|
        @modname = name
      end

      parser.on("-v MODULE_VERSION", "Module version") do |ver|
        @modver = ver
      end

      parser.on("-o OUTPUT", "Output file") do |out|
        out_file = out
      end

    end.parse!

    @in_files = ARGV

    raise StandardError.new("Missing input file(s)") if @in_files.empty?

    if out_file
      @out = File.open(out_file, "w")
    else
      @out = STDOUT
    end
  end
end

def hash_merge(h0, h1)
  merger = proc do |k, oldval, newval|
    if oldval.is_a?(Hash) and newval.is_a?(Hash)
      oldval.merge(newval, &merger)
    else
      newval
    end
  end

  h0.merge(h1, &merger)
end

GENERIC_MODULE_YML='
type: generic-module
description: Generic Rabbits module
parameters:
  debug:
    type: boolean
    description: Set log level to `debug\' for this module
    default: false
    advanced: true
  trace:
    type: boolean
    description: Set log level to `trace\' for this module
    default: false
    advanced: true
  log-target:
    type: string
    description: Specify the log target for this module (valid options are `stdout\', `stderr\' and `file\')
    default: stderr
    advanced: true
  log-level:
    type: string
    description: Specify the log level for this module (valid options are `trace\', `debug\', `info\', `warning\', `error\')
    default: info
    advanced: true
  log-file:
    type: string
    description: Specify the log file for this module
    default: module.log
    advanced: true
'

def load_yml(f)
  yml = Psych.load_file(f)

  case yml['include']
  when String, Integer
    inc = [ yml['include'] ]
  when Array
    inc = yml['include']
  when NilClass
    inc = []
  else
    raise StandardError.new('Invalid include directive in file ' + f)
  end

  incs_yml = {}

  inc.each do |inc_f|
    abs_inc_f = File::join(File::dirname(f), inc_f)
    incs_yml = hash_merge(incs_yml, load_yml(abs_inc_f))
  end

  yml = hash_merge(incs_yml, yml)

  yml.delete("include")

  yml
end

begin
  args = ParseArgs.new

  generic_module = Psych.load(GENERIC_MODULE_YML)

  comps = args.in_files.collect do |f|
    yml = load_yml(f)
    case yml.keys.first
    when "component"
      yml = hash_merge({"component" => generic_module}, yml)
      Component.new(f, yml['component'], args)
    when "backend"
      yml = hash_merge({"backend" => generic_module}, yml)
      Backend.new(f, yml['backend'], args)
    when "plugin"
      yml = hash_merge({"plugin" => generic_module}, yml)
      Plugin.new(f, yml['plugin'], args)
    else
      raise StandardError.new('Invalid description file ' + f)
    end
  end

  args.out.puts '/* Auto-generated. Modifications will be overwritten */'

  case args.mode
  when :factory
    args.out.puts comps.first.gen_factory

  when :static
    args.out.puts LoaderGenerator.new(comps, STATIC_LOADER_TPL, args).generate

  when :dynamic
    args.out.puts LoaderGenerator.new(comps, DYNAMIC_LOADER_TPL, args).generate
  end

  args.out.close if args.out != STDOUT

#rescue Exception => e
  #STDERR.puts e.message
  #exit 1
end

