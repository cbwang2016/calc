/**
语法树
 A*B+C的语法树：
      +
     / \
    /   \
   *     C
  / \
 A   B

 */
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>

#define TYPE_ID_PLUS 0
#define TYPE_ID_MINUS 1
#define TYPE_ID_LEFT_BRACKET 2
#define TYPE_ID_RIGHT_BRACKET 3
#define TYPE_ID_TIMES 4
#define TYPE_ID_DIVIDE 5
#define TYPE_ID_VALUE 6
#define TYPE_ID_POWER 7
#define TYPE_ID_FACTORIAL 8

#define FACTORIAL_MAX 170
using namespace std;

class SyntaxTree {
private:
    class OperatorNode {
    public:
        virtual unsigned int getOperatorTypeID() = 0;

        virtual double getValue() = 0;

        virtual bool isFull() = 0;

        virtual ~OperatorNode() = default;

        // 优先级：越大优先级越低
        int getOperatorPrecedence() {
            int type = getOperatorTypeID();
            switch (type) {
                case TYPE_ID_LEFT_BRACKET:
                case TYPE_ID_RIGHT_BRACKET:
                    return 0;
                case TYPE_ID_VALUE:
                    return 1;
                case TYPE_ID_FACTORIAL:
                    return 3;
                case TYPE_ID_POWER:
                    return 4;
                case TYPE_ID_TIMES:
                case TYPE_ID_DIVIDE:
                    return 5;
                case TYPE_ID_PLUS:
                case TYPE_ID_MINUS:
                    return 6;
                default:
                    throw runtime_error{"Unknown Operator Type"};
            }
        }

        /**
        *
        * @param node
        * @return false if failed
        */
        virtual bool addChild(OperatorNode *node) {
            if (hasOpenLeftBracket(this)) {
                return children.back()->addChild(node);
            }
            if (node->getOperatorPrecedence() >= this->getOperatorPrecedence() &&
                node->getOperatorTypeID() != TYPE_ID_VALUE) {
                if (this->parentNode == nullptr)
                    return false;
                node->parentNode = this->parentNode;
                this->parentNode->children.pop_back();
                this->parentNode->children.push_back(node);

                node->children.push_back(this);
                this->parentNode = node;
                return true;
            } else {
                if (isFull())
                    return children.back()->addChild(node);
                if (node->getOperatorTypeID() == TYPE_ID_RIGHT_BRACKET || this->getOperatorTypeID() == TYPE_ID_VALUE)
                    return false;
                node->parentNode = this;
                children.push_back(node);
                return true;
            }
        }

        vector<OperatorNode *> children{};
        OperatorNode *parentNode = nullptr;
    };

    class OperatorValue : public OperatorNode {
    public:
        explicit OperatorValue(double value) : value(value) {}

        unsigned int getOperatorTypeID() override { return TYPE_ID_VALUE; }

        double getValue() override { return value; }

        bool isFull() override { return false; }

    private:
        double value;
    };

    class OperatorLeftBracket : public OperatorNode {
    public:
        explicit OperatorLeftBracket() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_LEFT_BRACKET; }

        double getValue() override {
            if (children.empty() || children.back()->getOperatorTypeID() != TYPE_ID_RIGHT_BRACKET)
                throw runtime_error{"\")\" expected"};
            return children[0]->getValue();
        }

        bool isFull() override { return hasRightBracketAttached; }

        bool addChild(OperatorNode *node) override {
            if (node->getOperatorTypeID() == TYPE_ID_RIGHT_BRACKET) {
                if (hasRightBracketAttached)
                    return false;
                if (children.empty() || !children[0]->addChild(node)) {
                    node->parentNode = this;
                    children.push_back(node);
                    hasRightBracketAttached = true;
                }
                return true;
            }
            if (children.empty()) {
                if (node->getOperatorTypeID() == TYPE_ID_MINUS) {
                    this->addChild(new OperatorValue(0));
                    return OperatorNode::addChild(node);
                }
                node->parentNode = this;
                children.push_back(node);
                return true;
            }
            return OperatorNode::addChild(node);
        }

    private:
        bool hasRightBracketAttached = false;
    };

    class OperatorRightBracket : public OperatorNode {
    public:
        OperatorRightBracket() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_RIGHT_BRACKET; }

        double getValue() override { return 0; }

        bool isFull() override { return true; }

        bool addChild(OperatorNode *node) override { return false; }
    };

    class OperatorPlus : public OperatorNode {
    public:
        OperatorPlus() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_PLUS; }

        double getValue() override { return children.at(0)->getValue() + children.at(1)->getValue(); }

        bool isFull() override { return children.size() == 2; }
    };

    class OperatorMinus : public OperatorNode {
    public:
        OperatorMinus() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_MINUS; }

        double getValue() override { return children.at(0)->getValue() - children.at(1)->getValue(); }

        bool isFull() override { return children.size() == 2; }
    };

    class OperatorTimes : public OperatorNode {
    public:
        OperatorTimes() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_TIMES; }

        double getValue() override { return children.at(0)->getValue() * children.at(1)->getValue(); }

        bool isFull() override { return children.size() == 2; }
    };

    class OperatorDivide : public OperatorNode {
    public:
        OperatorDivide() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_DIVIDE; }

        double getValue() override {
            double d = children.at(1)->getValue();
            if (d == 0)
                throw runtime_error{"divide by zero"};
            return children.at(0)->getValue() / d;
        }

        bool isFull() override { return children.size() == 2; }
    };

    class OperatorPower : public OperatorNode {
    public:
        OperatorPower() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_POWER; }

        double getValue() override { return pow(children.at(0)->getValue(), children.at(1)->getValue()); }

        bool isFull() override { return children.size() == 2; }
    };

    class OperatorFactorial : public OperatorNode {
    public:
        OperatorFactorial() = default;

        unsigned int getOperatorTypeID() override { return TYPE_ID_FACTORIAL; }

        double getValue() override {
            double n = children.at(0)->getValue();
            double intpart;
            if (modf(n, &intpart) == 0.0 && intpart >= 0 && intpart < FACTORIAL_MAX) {
                return fact(intpart);
            } else
                throw runtime_error{"invalid value for factorial"};
        }

        bool isFull() override { return children.size() == 1; }

    private:
        double fact(double n) { return n == 0 ? 1 : n * fact(n - 1); }
    };

    static bool hasOpenLeftBracket(OperatorNode *node) {
        if (node->getOperatorTypeID() == TYPE_ID_LEFT_BRACKET && !node->isFull())
            return true;
        for (OperatorNode *tmp : node->children)
            if (hasOpenLeftBracket(tmp))
                return true;
        return false;
    }

    void deleteIterator(OperatorNode *node) {
        for (OperatorNode *i : node->children)
            deleteIterator(i);
        delete node;
    }

    OperatorNode *root = new OperatorLeftBracket();
public:
    SyntaxTree() = default;

    ~SyntaxTree() {
        deleteIterator(root);
    }

    bool parseFromStdIn() {
        while (true) {
            char ch{' '};
            cin.get(ch);
            bool flag{true};
            OperatorNode *tmp;
            switch (ch) {
                case '+':
                    tmp = new OperatorPlus();
                    flag = root->addChild(tmp);
                    break;
                case '-':
                    tmp = new OperatorMinus();
                    flag = root->addChild(tmp);
                    break;
                case '*':
                    tmp = new OperatorTimes();
                    flag = root->addChild(tmp);
                    break;
                case '/':
                    tmp = new OperatorDivide();
                    flag = root->addChild(tmp);
                    break;
                case '^':
                    tmp = new OperatorPower();
                    flag = root->addChild(tmp);
                    break;
                case '!':
                    tmp = new OperatorFactorial();
                    flag = root->addChild(tmp);
                    break;
                case '(':
                    tmp = new OperatorLeftBracket();
                    flag = root->addChild(tmp);
                    break;
                case ')':
                    tmp = new OperatorRightBracket();
                    flag = root->addChild(tmp);
                    break;
                case '.':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    cin.putback(ch);
                    double val{0};
                    cin >> val;
                    tmp = new OperatorValue(val);
                    flag = root->addChild(tmp);
                    break;
                }
                case '\n':
                case ';':
                    if (!root->children.empty()) {
                        try {
                            cout << " = " << root->children[0]->getValue() << endl;
                        } catch (out_of_range &e) {
                            cout << "syntax error" << endl;
                        } catch (runtime_error &e) {
                            cout << "Error when computing: " << e.what() << endl;
                        }
                    }
                    return true;
                default:
                    return false;
            }
            if (!flag) {
                delete tmp;
                cout << "Syntax error" << endl;
                return true;
            }
        }
    }
};

int main() {
    SyntaxTree *s = nullptr;
    do {
        cout << "> ";
        delete s;
        s = new SyntaxTree();
    } while (s->parseFromStdIn());
    delete s;
    return 0;
}