#ifndef VALUE_H
#define VALUE_H

#include <variant>
#include <string>
#include <vector>

class Value;
using Array = std::vector<Value>;

class Value
{
public:
    enum class Special {
        Undefined,
        Array,
    };
    Value();
    explicit Value(bool b);
    explicit Value(double d);
    explicit Value(std::string s);
    explicit Value(Special sp);

    std::string type() const;

    Value operator+(const Value & other);
    Value operator-(const Value & other);
    Value operator*(const Value & other);
    Value operator/(const Value & other);

    bool isNull() const;
    bool isBool() const;
    bool isDouble() const;
    bool isString() const;
    bool isUndefined() const;
    bool isArray() const;

    bool asBool() const;
    double asDouble() const;
    std::string asString() const;

    void setIndex(int index, Value cellValue);
    Value getIndex(int index) const;
    void push(Value val);
    Value pop();

    friend std::ostream& operator<<(std::ostream& os, const Value& v);
    friend bool operator==(const Value& l, const Value& r);
    friend bool operator<(const Value& l, const Value& r);

private:
    std::variant<std::nullptr_t, std::monostate, bool, double, std::string, Array> value;
};

bool operator>(const Value& l, const Value& r);
bool operator<=(const Value& l, const Value& r);
bool operator>=(const Value& l, const Value& r);
bool operator!=(const Value& l, const Value& r);

#endif // VALUE_H
