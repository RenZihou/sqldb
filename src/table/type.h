// type.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TYPE_H_
#define TYPE_H_

#include <string>

#include "../util/constants.h"

enum class ColumnType {
    INT,
    FLOAT,
    VARCHAR,
//    DATE,
    UNKNOWN,
};

class Type {
public:
    Type() = default;

    virtual ~Type() = default;

//    virtual BufType serialize() = 0;  // TODO

    [[nodiscard]] virtual ColumnType getType() const = 0;

    virtual void print() const = 0;

    virtual bool operator==(const Type &rhs) const = 0;

    virtual bool operator>(const Type &rhs) const = 0;

    virtual bool operator<(const Type &rhs) const = 0;

    virtual bool operator>=(const Type &rhs) const = 0;

    virtual bool operator<=(const Type &rhs) const = 0;

    virtual bool operator!=(const Type &rhs) const = 0;
};

class Int : public Type {
private:
    int value;
public:
    explicit Int(BufType buf) : Type(), value(*(int *) buf) {}

    [[nodiscard]] ColumnType getType() const override { return ColumnType::INT; }

    void print() const override { printf("%d", this->value); }

    bool operator==(const Type &rhs) const override {
        return rhs.getType() == ColumnType::INT
               && this->value == dynamic_cast<const Int *>(&rhs)->value;
    }

    bool operator>(const Type &rhs) const override {
        return rhs.getType() == ColumnType::INT
               && this->value > dynamic_cast<const Int *>(&rhs)->value;
    }

    bool operator<(const Type &rhs) const override {
        return rhs.getType() == ColumnType::INT
               && this->value < dynamic_cast<const Int *>(&rhs)->value;
    }

    bool operator>=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::INT
               && this->value >= dynamic_cast<const Int *>(&rhs)->value;
    }

    bool operator<=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::INT
               && this->value <= dynamic_cast<const Int *>(&rhs)->value;
    }

    bool operator!=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::INT
               && this->value != dynamic_cast<const Int *>(&rhs)->value;
    }
};

class Float : public Type {
private:
    float value;
public:
    explicit Float(BufType buf) : Type(), value(*(float *) buf) {}

    [[nodiscard]] ColumnType getType() const override { return ColumnType::FLOAT; }

    void print() const override { printf("%f", this->value); }

    bool operator==(const Type &rhs) const override {
        return rhs.getType() == ColumnType::FLOAT
               && this->value == dynamic_cast<const Float *>(&rhs)->value;
    }

    bool operator>(const Type &rhs) const override {
        return rhs.getType() == ColumnType::FLOAT
               && this->value > dynamic_cast<const Float *>(&rhs)->value;
    }

    bool operator<(const Type &rhs) const override {
        return rhs.getType() == ColumnType::FLOAT
               && this->value < dynamic_cast<const Float *>(&rhs)->value;
    }

    bool operator>=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::FLOAT
               && this->value >= dynamic_cast<const Float *>(&rhs)->value;
    }

    bool operator<=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::FLOAT
               && this->value <= dynamic_cast<const Float *>(&rhs)->value;
    }

    bool operator!=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::FLOAT
               && this->value != dynamic_cast<const Float *>(&rhs)->value;
    }
};

class VarChar : public Type {
private:
    std::string value;
public:
    VarChar(BufType buf, unsigned length) : Type(), value((char *) buf, length) {}

    [[nodiscard]] ColumnType getType() const override { return ColumnType::VARCHAR; }

    void print() const override { printf("%s", this->value.c_str()); }

    bool operator==(const Type &rhs) const override {
        return rhs.getType() == ColumnType::VARCHAR
               && this->value == dynamic_cast<const VarChar *>(&rhs)->value;
    }

    bool operator>(const Type &rhs) const override {
        return rhs.getType() == ColumnType::VARCHAR
               && this->value > dynamic_cast<const VarChar *>(&rhs)->value;
    }

    bool operator<(const Type &rhs) const override {
        return rhs.getType() == ColumnType::VARCHAR
               && this->value < dynamic_cast<const VarChar *>(&rhs)->value;
    }

    bool operator>=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::VARCHAR
               && this->value >= dynamic_cast<const VarChar *>(&rhs)->value;
    }

    bool operator<=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::VARCHAR
               && this->value <= dynamic_cast<const VarChar *>(&rhs)->value;
    }

    bool operator!=(const Type &rhs) const override {
        return rhs.getType() == ColumnType::VARCHAR
               && this->value != dynamic_cast<const VarChar *>(&rhs)->value;
    }
};

/**
 * @param buffer buffer stores the serialized data
 * @param type value type (column type)
 * @param length data length (number of bytes to read from buffer)
 * @return typed data
 */
Type *deserialize(BufType buffer, ColumnType type, unsigned length);

/**
 * @param value value in string format
 * @param type value type (column type)
 * @param buffer buffer to write serialized data
 * @param length data length (number of bytes to be written in buffer)
 * @description serializeFromString value to byte-form buffer
 */
void serializeFromString(const std::string &value, ColumnType type,
                         BufType buffer, unsigned length);

#endif  // TYPE_H_
