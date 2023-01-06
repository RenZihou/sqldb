// type.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TYPE_H_
#define TYPE_H_

#include <string>
#include <utility>
#include <cstring>
#include <iostream>

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

    virtual void serialize(BufType buffer, unsigned length) const = 0;

    [[nodiscard]] virtual ColumnType getType() const = 0;

    [[nodiscard]] virtual std::string toString() const = 0;

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

    explicit Int(const std::string& expr) : Type(), value(std::stoi(expr)) {}

    void serialize(BufType buffer, unsigned length) const override { *(int *) buffer = this->value; }

    [[nodiscard]] ColumnType getType() const override { return ColumnType::INT; }

    [[nodiscard]] int getValue() const { return this->value; }

    [[nodiscard]] std::string toString() const override { return std::to_string(this->value); }

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

    explicit Float(const std::string &expr) : Type(), value(std::stof(expr)) {}

    void serialize(BufType buffer, unsigned length) const override { *(float *) buffer = this->value;}

    [[nodiscard]] ColumnType getType() const override { return ColumnType::FLOAT; }

    [[nodiscard]] std::string toString() const override { return std::to_string(this->value); }

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
    explicit VarChar(BufType buf, unsigned length) : Type(), value((char *) buf) {}

    explicit VarChar(std::string expr) : Type(), value(std::move(expr)) {}

    void serialize(BufType buffer, unsigned length) const override {
        if (value.length() > length - 1) {
            std::cerr << "WARNING: value too long (" << value.length() << ") for column" << std::endl;
        } else {
            length = value.length();
        }
        memcpy(buffer, value.c_str(), length);
        buffer[length] = '\0';
    }

    [[nodiscard]] ColumnType getType() const override { return ColumnType::VARCHAR; }

    [[nodiscard]] std::string toString() const override { return this->value; }

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

class Null : public Type {
public:
    explicit Null() : Type() {}

    [[nodiscard]] ColumnType getType() const override { return ColumnType::UNKNOWN; }

    [[nodiscard]] std::string toString() const override { return "NULL"; }

    void serialize(BufType buffer, unsigned length) const override {}

    bool operator==(const Type &rhs) const override {
        return false;
    }

    bool operator>(const Type &rhs) const override {
        return false;
    }

    bool operator<(const Type &rhs) const override {
        return true;
    }

    bool operator>=(const Type &rhs) const override {
        return false;
    }

    bool operator<=(const Type &rhs) const override {
        return true;
    }

    bool operator!=(const Type &rhs) const override {
        return true;
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
 * @brief serializeFromString value to byte-form buffer
 * @param value value in string format
 * @param type value type (column type)
 * @param buffer buffer to write serialized data
 * @param length data length (number of bytes to be written in buffer)
 */
[[deprecated]] void serializeFromString(const std::string &value, ColumnType type,
                         BufType buffer, unsigned length);

#endif  // TYPE_H_
