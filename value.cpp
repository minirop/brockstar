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
    if (isNull()) return false;
    if (isArray()) return std::get<Array>(value).size() != 0;
    return std::get<bool>(value);
}

double Value::asDouble() const
{
    if (isBool()) return asBool() ? 1.0 : 0.0;
    if (isString()) return 0.0;
    if (isNull()) return 0.0;
    if (isArray()) return std::get<Array>(value).size();
    return std::get<double>(value);
}

std::string Value::asString() const
{
    if (isBool()) return asBool() ? "true"s : "false"s;
    if (isDouble()) return std::to_string(asDouble());
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