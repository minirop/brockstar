#include "value.h"
#include <ostream>
#include <cmath>

#include <iostream>

using namespace std::string_literals;

Value::Value()
    : value { nullptr }
{
}

Value::Value(bool b)
    : value { b }
{
}

Value::Value(double d)
    : value { d }
{
}

Value::Value(std::string s)
    : value { s }
{
}

Value::Value(Special sp)
{
    switch (sp)
    {
    case Special::Undefined:
        value = std::monostate{};
        break;
    case Special::Array:
        value = Array{};
        break;
    default:
        throw 42;
    }
}

std::string Value::type() const
{
    if (isNull()) return "null"s;
    if (isBool()) return "bool"s;
    if (isDouble()) return "number"s;
    if (isString()) return "string"s;
    if (isUndefined()) return "mysterious"s;
    if (isArray()) return "array"s;
    return "WHAT?!!"s;
}

Value Value::operator+(const Value & other)
{
    if (isString() || other.isString())
    {
        return Value(asString() + other.asString());
    }

    return Value(asDouble() + other.asDouble());
}

Value Value::operator-(const Value & other)
{
    return Value(asDouble() - other.asDouble());
}

Value Value::operator*(const Value & other)
{
    if (isString())
    {
        if (other.isString())
        {
            return Value(Value::Special::Undefined);
        }

        std::string res;
        for (int i = 0; i < (int)floor(other.asDouble()); i++)
        {
            res += asString();
        }

        return Value(res);
    }
    return Value(asDouble() * other.asDouble());
}

Value Value::operator/(const Value & other)
{
    return Value(asDouble() / other.asDouble());
}

bool Value::isNull() const
{
    return std::holds_alternative<std::nullptr_t>(value);
}

bool Value::isBool() const
{
    return std::holds_alternative<bool>(value);
}

bool Value::isDouble() const
{
    return std::holds_alternative<double>(value);
}

bool Value::isString() const
{
    return std::holds_alternative<std::string>(value);
}

bool Value::isUndefined() const
{
    return std::holds_alternative<std::monostate>(value);
}

bool Value::isArray() const
{
    return std::holds_alternative<Array>(value);
}

bool Value::asBool() const
{
    if (isDouble()) return asDouble() != 0.0;
    if (isString()) return asString().size() != 0;
    if (isNull() || isUndefined()) return false;
    if (isArray()) return std::get<Array>(value).size() != 0;
    return std::get<bool>(value);
}

double Value::asDouble() const
{
    if (isBool()) return asBool() ? 1.0 : 0.0;
    if (isString()) return 0.0;
    if (isNull() || isUndefined()) return 0.0;
    if (isArray()) return std::get<Array>(value).size();
    return std::get<double>(value);
}

std::string Value::asString() const
{
    if (isBool()) return asBool() ? "true"s : "false"s;
    if (isDouble())
    {
        auto str = std::to_string(asDouble());
        while (str.ends_with("0"))
        {
            str.pop_back();
        }

        if (str.ends_with(".")) str.pop_back();

        return str;
    }
    if (isNull()) return "null"s;
    if (isUndefined()) return "mysterious"s;
    if (isArray()) return "Array";
    return std::get<std::string>(value);
}

void Value::setIndex(int index, Value cellValue)
{
    if (!isArray())
    {
        std::cerr << "Can't index a variable of type " << type() << ", must be an array.\n";
        std::exit(1);
    }

    auto & content = std::get<Array>(value);
    if (static_cast<int>(content.size()) <= index)
    {
        content.resize(index + 1);
        content[index] = cellValue;
    }
}

Value Value::getIndex(int index) const
{
    if (!isArray() && !isString())
    {
        std::cerr << "Can't index a variable of type " << type() << ", must be an array or a string.\n";
        return Value(Special::Undefined);
    }

    if (isArray())
    {
        auto & content = std::get<Array>(value);
        if (index < static_cast<int>(content.size()))
        {
            return content[index];
        }
        else
        {
            std::cerr << "Index out of bound, " << index << " >= " << content.size() << ".\n";
            std::exit(1);
        }
    }
    else // isString()
    {
        auto str = asString();

        if (index < static_cast<int>(str.size()))
        {
            return Value(""s + str[index]);
        }
        else
        {
            std::cerr << "Index out of bound, " << index << " >= " << str.size() << ".\n";
            std::exit(1);
        }
    }
}

void Value::push(Value val)
{
    if (!isArray())
    {
        value = Array();
    }

    auto & arr = std::get<Array>(value);
    arr.push_back(val);
}

Value Value::pop()
{
    if (!isArray())
    {
        std::cerr << "Can't pop value from non-array\n";
        std::exit(1);
    }

    auto & arr = std::get<Array>(value);
    if (arr.size() == 0)
    {
        return Value(Value::Special::Undefined);
    }

    auto val = arr.front();
    arr.erase(arr.begin());
    return val;
}

std::ostream& operator<<(std::ostream& os, const Value& v)
{
    if (v.isDouble() || v.isArray()) os << v.asDouble();
    else os << v.asString();

    return os;
}

bool operator==(const Value& l, const Value& r)
{
    if (l.type() == r.type())
    {
        if (l.isUndefined() || l.isNull())
        {
            return true;
        }

        if (l.isDouble())
        {
            return l.asDouble() == r.asDouble();
        }

        if (l.isBool())
        {
            return l.asBool() == r.asBool();
        }

        if (l.isArray())
        {
            // check their sizes
            if (l.asDouble() != r.asDouble()) return false;

            const auto & arr1 = std::get<Array>(l.value);
            const auto & arr2 = std::get<Array>(r.value);

            for (size_t i = 0; i < arr1.size(); i++)
            {
                if (arr1[i] != arr2[i])
                {
                    return false;
                }
            }
            return true;
        }

        if (l.isString())
        {
            return l.asString() == r.asString();
        }
    }

    if ((l.isString() && r.isDouble()) || (l.isDouble() && r.isString()))
    {
        return l.asDouble() == r.asDouble();
    }

    if ((l.isString() && r.isBool()) || (l.isBool() && r.isString()))
    {
        return l.asBool() == r.asBool();
    }

    if ((l.isString() && r.isNull()) || (l.isNull() && r.isString()))
    {
        return false;
    }

    if ((l.isDouble() && r.isBool()) || (l.isBool() && r.isDouble()))
    {
        return l.asBool() == r.asBool();
    }

    if ((l.isDouble() && r.isNull()) || (l.isNull() && r.isDouble()))
    {
        return l.asBool() == r.asBool();
    }

    if ((l.isNull() && r.isBool()) || (l.isBool() && r.isNull()))
    {
        return l.asBool() == r.asBool();
    }

    return false;
}

bool operator<(const Value& l, const Value& r)
{
    if (l.isUndefined() || r.isUndefined()) return false;

    if (l.isDouble() && r.isDouble()) return l.asDouble() < r.asDouble();

    if ((l.isNull() && r.isDouble()) || (l.isDouble() && r.isNull()))
    {
        return l.asDouble() < r.asDouble();
    }

    if ((l.isString() && r.isDouble()) || (l.isDouble() && r.isString()))
    {
        return l.asDouble() < r.asDouble();
    }

    if (l.isString() && r.isString())
    {
        return l.asString() < r.asString();
    }

    if (l.isBool() || r.isBool())
    {
        throw "Can't order booleans";
    }

    return false;
}

bool operator>(const Value& l, const Value& r)
{
    return r < l;
}

bool operator<=(const Value& l, const Value& r)
{
    return l < r || l == r;
}

bool operator>=(const Value& l, const Value& r)
{
    return l > r || l == r;
}

bool operator!=(const Value& l, const Value& r)
{
    return !(l == r);
}
