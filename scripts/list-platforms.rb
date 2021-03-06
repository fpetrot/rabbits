# This file is part of Rabbits
# Copyright (C) 2017  Clement Deschamps and Luc Michel
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

platforms = []

ARGV.each do |f|
  begin
    yml = Psych.load_file(f)

    yml['platforms'].each do |p,y|
      platforms.push(p) unless y['generic']
    end if yml.has_key?('platforms')
  rescue Exception => e
    STDERR.puts e.message
    exit 1
  end
end

prefix = ''
platforms.uniq.each do |p|
  STDOUT.printf("%s%s", prefix, p)
  prefix = ';'
end
