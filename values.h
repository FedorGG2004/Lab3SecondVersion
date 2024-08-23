#pragma once

#include <iostream>
#include <ostream>
#include <utility>
#include <vector>
#include "bases.h"


enum class CurType {
    INT,
    SHORT,
    BOOL
};

class Value {
    CurType m_type = CurType::INT;
    int m_value = 0;
    bool is_return = 0;
public:

    CurType type() const { return m_type; }
    void setType(CurType type) { m_type = type; }

    void setRet() { is_return = 1; }
    bool getRet() const{ return is_return;}

    Value(CurType type) : m_type(type) {}

    BoolBase BoolValue() const {
        if (m_value < 0)
            return BoolBase::FALSE;
        else if(m_value > 0)
            return BoolBase::TRUE;
        else
            return BoolBase::UNDEF;
    }
    void printType(std::string name){
        switch (m_type) {
            case CurType::INT:
                std::cout << name << " size: 4" << std::endl; 
            break;
            case CurType::SHORT:
                std::cout << name << " size: 2" << std::endl; 
            break;
            case CurType::BOOL:
                std::cout << name << " size: 1" << std::endl; 
        }
    }
    CurType getType() const {
        return m_type;
    }
//  CTORS

    Value() {}

//  int
    Value(int new_value) : m_type(CurType::INT), m_value(new_value) {}

    Value(short new_value) : m_type(CurType::SHORT), m_value(new_value) {}

    Value(bool new_value) : m_type(CurType::BOOL) {
        if(new_value == true){
            m_value = 1;
        }else if(new_value == false){
            m_value = -1;
        }
    }

    // Value(IntBase type) : m_type(CurType::INT), m_int(type) {}

//  bool
    Value(BoolBase type) : m_type(CurType::BOOL) {
        if (type == BoolBase::TRUE) {
            m_value = 1;
        }
        else if(type == BoolBase::FALSE){
            m_value = -1;
        }
        else{
            m_value = 0;
        }
    }

//  operators

    Value operator+(const Value& other) const {
        return m_value + other.m_value;
    }

    void operator+=(const Value& other) {
        m_value += other.m_value;
    }

    Value operator-(const Value& other) const {
        return m_value - other.m_value;
    }

    Value operator-() {
        return -m_value;
    }

    bool operator>(const Value& other) const {
        return m_value > other.m_value;
    }

    bool operator<(const Value& other) const {
        return m_value < other.m_value;
    }

    bool operator==(const Value& other) const {
        return m_value == other.m_value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Value& integer)  {
        
        switch (integer.m_type) {
            case CurType::INT:
                os << integer.m_value; 
            break;
            case CurType::SHORT:
                os << integer.m_value; 
            break;
            case CurType::BOOL:
                if(integer.m_value > 0)
                    os << "true";
                else if(integer.m_value < 0)
                    os << "false";
                else
                    os << "undef";
        }
        return os; 
    }

};


class ValueVector : public Value {
private:
    std::vector<Value> m_vector;

public:
    ValueVector(CurType type) : Value(type) {}

    // Добавление элемента в вектор
    void push_back(const Value& value) {
        m_vector.push_back(value);
    }

    // Получение элемента по индексу
    const Value& operator[](size_t index) const {
        return m_vector[index];
    }

    // Получение элемента по индексу (изменяемая версия)
    Value& operator[](size_t index) {
        return m_vector[index];
    }

    // Размер вектора
    size_t size() const {
        return m_vector.size();
    }

    // Оператор вывода
    friend std::ostream& operator<<(std::ostream& os, const ValueVector& vector) {
        os << "[";
        for (size_t i = 0; i < vector.size(); ++i) {
            os << vector[i];
            if (i != vector.size() - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
};