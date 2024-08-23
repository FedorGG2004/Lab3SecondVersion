#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <map>
#include <stack>
#include <string>

#include "values.h"
#include "bases.h"


class varTable {
std::map<std::string, Value> varMap;

public:
    void setVar(const std::string &name, const Value &value) {            
        varMap[name] = value;
    }

    auto getVar(const std::string &name) {
        return varMap[name];
    }

    void print() {
        std::cout << "Local in function variables:" << std::endl;
        for (auto &it : varMap) {
            std::cout << it.first << ' ' << it.second << std::endl;
        }
    }

};

// Define the abstract syntax tree (AST) nodes
class Node {
public:
    virtual Value eval(varTable&) { return Value(); }
};

typedef std::vector<Node*> nodeList;

inline Value GenericRet() {
    auto tmp = Value(); 
    tmp.setRet(); 
    return tmp;
}

class RetNode : public Node {
    Value eval(varTable&) { return GenericRet(); }
};

class SizeofNode : public Node {
public:
    SizeofNode(const std::string &name) : m_name(name) {}
    // SizeofNode(const CurType &type) : m_type(type) {}
    Value eval(varTable& vm) override { 
        vm.getVar(m_name).printType(m_name);    
        return Value();
    }
private:
    std::string m_name;
    // CurType m_type;
};

class VectorNode : public Node {
public:
    VectorNode() {}
    VectorNode(const Value &value){
        m_vec.push_back(value);
    }
    VectorNode(const Value &value, const std::string &name) {
        m_vec.push_back(value);
        m_name = name;
    }

    Value eval(varTable& vm) override { 
        if (m_name.empty())  //  immediate
            return Value(m_vec[0]); 

        //  variable
        return vm.getVar(m_name);
    }
private:
    std::vector<Value> m_vec;
    std::string m_name;
};

class VectorIndexNode : public Node {
public:
    VectorIndexNode(const Value &value) : m_value(value) {}
    VectorIndexNode(const Value &value, const std::string &name) : m_value(value), m_name(name) {}

    Value eval(varTable& vm) override { 
        return vm.getVar(m_name);
    }
private:
    Value m_value;
    std::string m_name;
};

class NumberNode : public Node {
public:
    NumberNode() {}
    NumberNode(const Value &value) : m_value(value) {}
    NumberNode(const Value &value, const std::string &name) : m_value(value), m_name(name) {}

    Value eval(varTable& vm) override { 
        if (m_name.empty())  //  immediate
            return Value(m_value); 

        //  variable
        return vm.getVar(m_name);
    }
private:
    Value m_value;
    std::string m_name;
};

class BinaryOpNode : public Node {
public:
    BinaryOpNode(char op, Node* left, Node* right) : op(op), left(left), right(right) {}
    Value eval(varTable& vm) override {
        switch (op) {
            // case '&': 
            //     return left->eval(vm) && right->eval(vm);
            // case '|': 
            //     return left->eval(vm) || right->eval(vm);
            case '+': 
                return left->eval(vm) + right->eval(vm);
            case '-': 
                return left->eval(vm) - right->eval(vm);
            case '<': 
                return left->eval(vm) < right->eval(vm);
            case '>': 
                return left->eval(vm) > right->eval(vm);
            case '=': 
                return left->eval(vm) == right->eval(vm);
            default: throw std::runtime_error("Invalid binary operator");
        }
    }
private:
    char op;
    Node* left;
    Node* right;
};

class UnaryMinusNode : public Node {
public:
    UnaryMinusNode(Node* operand) : operand(operand) {}
    Value eval(varTable& vm) override { return operand->eval(vm); }
private:
    Node* operand;
};

typedef std::pair<CurType, std::string> Signature;
typedef std::function<Value(Value, nodeList&, Signature)> FuncLambda;
typedef std::pair<FuncLambda, nodeList> Body;
typedef std::pair<Signature, Body> FuncDtor;

class funcTable;

class FunctionCallNode : public Node {
public:
    FunctionCallNode(const std::string &name, Node *arg, funcTable &table);
    Value eval(varTable& vm) override;

private:
    FuncDtor m_fdtor;
    funcTable &m_table;
    Node *m_arg;
    std::string m_name;
};

class funcTable {
std::map<std::string, FuncDtor> funcMap;

public:
    void setFuncSign(std::string name, Signature sign) {
        funcMap[name] = std::make_pair(sign, std::make_pair(FuncLambda(), nodeList()));
    }
    void setFuncBody(std::string name, Body body) {
        auto it = funcMap.find(name);
        if (it == funcMap.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        funcMap[name].second = body;
    }

    auto getFunc(const std::string &name) {
        auto it = funcMap.find(name);

        if (it == funcMap.end()) {
            throw std::runtime_error("Function not found: " + name);
        }

        return it->second;
    }

    void print() {
        std::cout << "!Func map:!" << std::endl;
        for (const auto &entry : funcMap) {
            std::cout << entry.first << ' ' << std::endl;
        }
    }    
};

class SetNode : public Node {

public:
    SetNode(const std::string &name, Node* arg) : m_name(name), m_arg(arg) {}
    Value eval(varTable& vm) override {
        // Look up the variable in the variable table
        vm.setVar(m_name, m_arg->eval(vm));
        return Value();
    }

    std::string m_name;
    Node* m_arg;
};

class DoWhileNode : public Node {
public:
    DoWhileNode(Node* condition, nodeList body) : m_condition(condition), m_body(body) {}

    Value eval(varTable& vm) override {

        if (m_condition->eval(vm).BoolValue() == BoolBase::UNDEF) 
            return Value();

        while (m_condition->eval(vm).BoolValue() == BoolBase::TRUE) {            
            for (auto &stmt : m_body) {
                if (stmt->eval(vm).getRet()) return GenericRet();
            }
        }

        return Value(); // Return value
    }
private:
    Node* m_condition;
    nodeList m_body;
};

class IfNode : public Node {
public:
    IfNode(Node* condition, nodeList trueBlock, nodeList elseBlock) : condition(condition), m_true(trueBlock), m_else(elseBlock) {}

    IfNode(Node* condition, nodeList trueBlock) : condition(condition), m_true(trueBlock) {}

    Value eval(varTable& vm) override {
        switch (condition->eval(vm).BoolValue()) {
            case BoolBase::TRUE:
                for (auto &stmt : m_true) {
                    if (stmt->eval(vm).getRet()) return GenericRet();
                }
                break;
            case BoolBase::FALSE:
                for (auto &stmt : m_else) {
                    if (stmt->eval(vm).getRet()) return GenericRet();
                }
                break;
            case BoolBase::UNDEF:
                for (auto &stmt : m_else) {
                    if (stmt->eval(vm).getRet()) return GenericRet();
                }
                break;
        }
        return Value(); // Return a dummy value
    }
private:
    Node* condition;
    nodeList m_true;
    nodeList m_else;
};

