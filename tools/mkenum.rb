#!/usr/bin/env ruby

BASE = <<EOT
class #NAME#
{
public:
#OPTIONLIST#

        #NAME# operator|(#NAME# other) const { return #NAME#(m_value | other.m_value); }
        #NAME# operator&(#NAME# other) const { return #NAME#(m_value & other.m_value); }
        #NAME# operator^(#NAME# other) const { return #NAME#(m_value ^ other.m_value); }
        #NAME#& operator|=(#NAME# other) { m_value |= other.m_value; return *this; }
        #NAME#& operator&=(#NAME# other) { m_value &= other.m_value; return *this; }
        #NAME#& operator^=(#NAME# other) { m_value ^= other.m_value; return *this; }
        #NAME# operator~() const { return #NAME#(~m_value); }

	operator bool() const { return m_value != NONE.m_value; }
	bool operator!() const { return m_value == NONE.m_value; }
        bool operator==(#NAME# other) const { return m_value == other.m_value; }
        bool operator!=(#NAME# other) const { return m_value != other.m_value; }

        unsigned int get_value() const { return m_value; }

protected:
        explicit #NAME#(unsigned int value) : m_value(value) { }

        unsigned int m_value;
};

#OPTIONASSIGNMENT#
EOT

raise Exception, "Not enough parameters specified!" unless ARGV.length > 0
name = ARGV[0]

optionlist = [['NONE', 0]]; cnt = 1/2.0
STDIN.each_line { |option| optionlist << [option.strip, (cnt *= 2).floor] }

puts BASE.gsub(/#(\w+)#/) { |token| 
  case token
    when '#NAME#': name
    when '#OPTIONLIST#': optionlist.map { |option|
        "\tstatic const #{name} #{option[0]};"
      }.join("\n")
    when '#OPTIONASSIGNMENT#': optionlist.map { |option|
        "const %s %s::%s = %s(0x%.8x);" % [name, name, option[0], name, option[1]]
      }.join("\n")
  end
}

# vim:set et ts=2:
