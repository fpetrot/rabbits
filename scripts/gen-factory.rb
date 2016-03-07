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

PLUGIN_HEADER_TPL='
#include <rabbits/plugin/factory.h>
%{include}

namespace autogen {

class %{factory_class}: public PluginFactory {
public:
    virtual ~%{factory_class}() {}

    Plugin * create() {
        return new %{class};
    }

    virtual std::string get_name() { return "%{name}"; }
};

}; /* namespace autogen */
'

PARAM_TPL=(' ' * 8) + 'add_param("%{name}", Parameter<%{type}>("%{description}", %{default}));'

DISCOVER_TPL=(' ' * 4) + 
   'virtual void discover(const std::string &name, const PlatformDescription &params) {
        ComponentParameters cp = get_params();
        cp.fill_from_description(params);
        
        %{discover_snippet}
    }
'

COMPONENT_HEADER_TPL='
#include <rabbits/component/factory.h>
%{include}

namespace autogen {

class %{factory_class} : public ComponentFactory {
public:
    %{factory_class}() {
%{parameters}
    }

    virtual ~%{factory_class}() {}

%{discover}
    ComponentBase * create(const std::string & name, const PlatformDescription &params) {
        ComponentParameters cp = get_params();
        cp.fill_from_description(params);
        
        return new %{class}(name.c_str(), cp);
    }

    virtual std::string name() { return "%{name}"; }
    virtual std::string type() { return "%{type}"; }
    virtual std::string description() { return "%{description}"; }
};

}; /* namespace autogen */
'

STATIC_INST_TPL='
%{self_include}
static autogen::%{factory_class} %{factory_class}_inst;
'


DYNAMIC_COMP_INST_TPL=(' ' * 8) + 'm_comp_insts.push_back(new autogen::%{factory_class});'
DYNAMIC_PLUGIN_INST_TPL=(' ' * 8) + 'm_plugin_insts.push_back(new autogen::%{factory_class});'

DYNAMIC_LOADER_TPL='
#include <vector>

#include <rabbits/config.h>
#include <rabbits/component/factory.h>
#include <rabbits/plugin/factory.h>
#include <rabbits/dynloader/dynloader.h>

%{includes}

namespace %{module_name}_autogen {
class RabbitsDynLoader;

static RabbitsDynLoader *inst = NULL;

class RabbitsDynLoader {
protected:
    std::vector<ComponentFactory*> m_comp_insts;
    std::vector<PluginFactory*> m_plugin_insts;

    void create_insts() { 
%{create_insts}
    }

    void destroy_insts() {
        std::vector<ComponentFactory*>::iterator cit;
        for (cit = m_comp_insts.begin(); cit != m_comp_insts.end(); cit++) {
            delete *cit;
        }
        std::vector<PluginFactory*>::iterator pit;
        for (pit = m_plugin_insts.begin(); pit != m_plugin_insts.end(); pit++) {
            delete *pit;
        }
    }

public:
    RabbitsDynLoader() { create_insts(); }
    virtual ~RabbitsDynLoader() { destroy_insts(); }

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

void rabbits_dynamic_load(void)
{
    if (%{module_name}_autogen::inst == NULL) {
        %{module_name}_autogen::inst = new %{module_name}_autogen::RabbitsDynLoader;
    }
}

void rabbits_dynamic_unload(void)
{
    delete %{module_name}_autogen::inst;
    %{module_name}_autogen::inst = NULL;
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
    return str if str.is_a?(Fixnum)

    unit = str[/-?[0-9]([kKmMgG])/,1]
    factor = 1

    case unit
    when 'k', 'K'
      factor = 1024
    when 'm', 'M'
      factor = 1024 * 1024
    when 'g', 'G'
      factor = 1024 * 1024 * 1024
    end

    return str.to_i * factor
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
    @cc_type = 'sc_time'
  end

  def convert(str)
    "todo"
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
DescrTypeBoolean.new('boolean')
DescrTypeTime.new('time')

class Parameter
  attr_accessor :name, :type, :default, :description

  def initialize(name, descr)
    required = ['type', 'default', 'description']

    raise StandardError.new("Missing attribute(s): " + (required - descr.keys).join(",") + " for parameter `#{name}'") if (descr.keys & required).sort != required.sort

    converter = DescrType.get(descr['type'])

    raise StandardError.new("Unknown type `#{descr['type']}' for parameter `#{name}'") unless converter

    @name = name
    @type = converter.cc_type
    @default = converter.convert(descr['default'])
    @description = descr['description'].gsub("\n", '\n')
  end

  def get_print_args
    {
      :name => @name,
      :description => @description,
      :type => @type,
      :default => @default
    }
  end
end

class Component
  attr_accessor :fn, :name, :type, :description, :parameters, :class, :include, :discover
  
  def initialize(fn, descr)
    required = ['name', 'type', 'description', 'class', 'include']

    raise StandardError.new("Missing attribute(s): " + (required - descr.keys).join(",")) if (descr.keys & required).sort != required.sort
    
    @fn = fn
    @name = descr['name']
    @type = descr['type']
    @description = descr['description'].gsub("\n", '\n')
    @class = descr['class']
    @include = descr['include']
    @discover = descr['discover']
    @parameters = {}

    descr['parameters'].each do |pname, pval|
      @parameters[pname] = Parameter.new(pname, pval)
    end if descr.key?('parameters')
  end

  def build_parameters
    @parameters.map do |_, p|
      PARAM_TPL % p.get_print_args
    end.join("\n")
  end

  def build_include
    if @include.is_a?(Array) then
      @include.map do |i|
        '#include "' + i + '"'
      end.join("\n")
    else
      "#include \"#{@include}\""
    end
  end

  def build_self_include
    '#include "' + @fn.gsub($source_dir, $bin_dir) + '.h"'
  end

  def build_discover
    if @discover then
      DISCOVER_TPL % { :discover_snippet => @discover + ';' }
    else
      ''
    end
  end

  def get_print_args
    { 
      :name => @name, 
      :type => @type,
      :description => @description,
      :class => @class,
      :factory_class => cc_ify(@name) + "Factory",
      :include => build_include,
      :self_include => build_self_include,
      :parameters => build_parameters,
      :discover => build_discover,
    }
  end

  def gen_factory
    COMPONENT_HEADER_TPL % get_print_args
  end

  def gen_static_inst
    STATIC_INST_TPL % get_print_args
  end

  def gen_dynamic_inst
    DYNAMIC_COMP_INST_TPL % get_print_args
  end
end

class Plugin
  attr_accessor :fn, :name, :class, :include
  
  def initialize(fn, descr)
    required = ['name', 'class', 'include']

    raise StandardError.new("Missing attribute(s): " + (required - descr.keys).join(",")) if (descr.keys & required).sort != required.sort
 
    @fn = fn
    @name = descr['name']
    @class = descr['class']
    @include = descr['include']
  end

  def build_include
    if @include.is_a?(Array) then
      @include.map do |i|
        '#include "' + i + '"'
      end.join("\n")
    else
      "#include \"#{@include}\""
    end
  end

  def build_self_include
    '#include "' + @fn.gsub($source_dir, $bin_dir) + '.h"'
  end

  def get_print_args
    {
      :name => @name,
      :class => @class,
      :factory_class => cc_ify(@name) + "Factory",
      :include => build_include,
      :self_include => build_self_include,
    }
  end

  def gen_factory
    PLUGIN_HEADER_TPL % get_print_args
  end

  def gen_static_inst
    STATIC_INST_TPL % get_print_args
  end

  def gen_dynamic_inst
    DYNAMIC_PLUGIN_INST_TPL % get_print_args
  end
end

def usage
  STDERR.puts "Usage: #{__FILE__} [--factory] [-o out] description.yml"
  STDERR.puts "       #{__FILE__} --static-insts|--dyn-loader -s src_dir -b bin_dir -m modname -v modver [-o out] description.yml [...]"
  exit 1
end

def parse_args
  mode = :factory
  state = :mode
  out_file = nil
  $source_dir = ""
  $bin_dir = ""
  in_files = []

  ARGV.each do |arg|
    case state
    when :mode
      case arg
      when "--factory"
        mode = :factory
        state = :files_or_out
      when "--static-insts"
        mode = :static
        state = :source_dir
      when "--dyn-loader"
        mode = :dynamic
        state = :source_dir
      else
        state = :files_or_out
        redo
      end

    when :source_dir
      if arg != "-s" then
        usage
        exit 1
      end
      state = :source_dir_val

    when :source_dir_val
      $source_dir = arg
      state = :bin_dir

    when :bin_dir
      if arg != "-b" then
        usage
        exit 1
      end
      state = :bin_dir_val

    when :bin_dir_val
      $bin_dir = arg
      state = :modname

    when :modname
      if arg != "-m" then
        usage
        exit 1
      end
      state = :modname_val

    when :modname_val
      $modname = arg
      state = :version

    when :version
      if arg != "-v"
        usage
        exit 1
      end
      state = :version_val

    when :version_val
      $modver = arg
      state = :files_or_out

    when :files_or_out
      case arg
      when "-o"
        state = :out_file
      else
        state = :files
        redo
      end

    when :out_file
      out_file = arg
      state = :files

    when :files
      in_files.push(arg)
    end
  end

  if in_files.empty? then
    usage
    exit 1
  end

  if mode == :factory and in_files.length > 1 then
    usage
    exit 1
  end

  if out_file then
    out = File.open(out_file, "w")
  else
    out = STDOUT
  end

  return [ mode, out, in_files ]
end

begin 
  mode, out, in_files = parse_args

  comps = in_files.collect do |f|
    yml = Psych.load_file(f)
    case yml.keys.first
    when "component"
      Component.new(f, yml['component'])
    when "plugin"
      Plugin.new(f, yml['plugin'])
    else
      raise StandardError.new('Invalid description file ' + f)
    end
  end

  out.puts '/* Auto-generated. Modifications will be overwritten */'

  case mode
  when :factory
    out.puts comps.first.gen_factory

  when :static
    comps.each do |c|
      out.puts c.gen_static_inst
    end

  when :dynamic
    insts = ""
    includes = ""
    comps.each do |c|
      insts += c.gen_dynamic_inst + "\n"
      includes += c.get_print_args[:self_include] + "\n"
    end

    out.puts DYNAMIC_LOADER_TPL % { 
      :includes => includes,
      :create_insts => insts,
      :module_name => $modname,
      :module_version => $modver
    }
  end

  out.close if out != STDOUT

#rescue Exception => e
  #STDERR.puts e.message
  #exit 1
end

