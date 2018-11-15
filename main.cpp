#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

class SyntaxTree {
private:
    class OperatorNode {
    public:
        virtual string getType() = 0;

        virtual double getValue() = 0;

        virtual bool isFull() = 0;

        virtual ~OperatorNode() = default;

        // 优先级：越大优先级越低
        int getOperatorPrecedence() {
            string type = getType();
            if (type == "(" || type == ")") {
                return 0;
            } else if (type == "value") {
                return 1;
            } else if (type == "^") {
                return 2;
            } else if (type == "*" || type == "/") {
                return 3;
            } else if (type == "+" || type == "-") {
                return 4;
            } else {
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
                return childs.back()->addChild(node);
            }
            if (node->getOperatorPrecedence() >= this->getOperatorPrecedence() && node->getType() != "value") {
                node->parentNode = this->parentNode;
                this->parentNode->childs.pop_back();
                this->parentNode->childs.push_back(node);

                node->childs.push_back(this);
                this->parentNode = node;
                return true;
            } else {
                if (isFull()) {
                    return childs.back()->addChild(node);
                }
                if (node->getType() == ")" || this->getType() == "value")
                    return false;
                node->parentNode = this;
                childs.push_back(node);
                return true;
            }
        }

        vector<OperatorNode *> childs{};
        OperatorNode *parentNode = nullptr;
    };

    class OperatorValue : public OperatorNode {
    public:
        explicit OperatorValue(double value) : value(value) {}

        string getType() override { return "value"; }

        double getValue() override { return value; }

        bool isFull() override { return false; }

    private:
        double value;
    };

    class OperatorLeftBracket : public OperatorNode {
    public:
        explicit OperatorLeftBracket() = default;

        string getType() override { return "("; }

        double getValue() override {
            if (childs.empty() || childs.back()->getType() != ")")
                throw runtime_error{"\")\" expected"};
            return childs[0]->getValue();
        }

        bool isFull() override { return hasRightBracketAttached; }

        bool addChild(OperatorNode *node) override {
            if (node->getType() == ")") {
                if (hasRightBracketAttached)
                    return false;
                if (childs.empty() || !childs[0]->addChild(node)) {
                    node->parentNode = this;
                    childs.push_back(node);
                    hasRightBracketAttached = true;
                    return true;
                } else {
                    return true;
                }
            }
            if (childs.empty()) {
                node->parentNode = this;
                childs.push_back(node);
                return true;
            } else if (isFull()) {
                return OperatorNode::addChild(node);
            }
            return childs[0]->addChild(node);
        }

    private:
        bool hasRightBracketAttached = false;
    };

    class OperatorRightBracket : public OperatorNode {
    public:
        OperatorRightBracket() = default;

        string getType() override { return ")"; }

        double getValue() override { return 0; }

        bool isFull() override { return true; }

        bool addChild(OperatorNode *node) override { return false; }
    };

    class OperatorPlus : public OperatorNode {
    public:
        OperatorPlus() = default;

        string getType() override { return "+"; }

        double getValue() override { return childs.at(0)->getValue() + childs.at(1)->getValue(); }

        bool isFull() override { return childs.size() == 2; }
    };

    class OperatorMinus : public OperatorNode {
    public:
        OperatorMinus() = default;

        string getType() override { return "-"; }

        double getValue() override { return childs.at(0)->getValue() - childs.at(1)->getValue(); }

        bool isFull() override { return childs.size() == 2; }
    };

    class OperatorTimes : public OperatorNode {
    public:
        OperatorTimes() = default;

        string getType() override { return "*"; }

        double getValue() override { return childs.at(0)->getValue() * childs.at(1)->getValue(); }

        bool isFull() override { return childs.size() == 2; }
    };

    class OperatorDivide : public OperatorNode {
    public:
        OperatorDivide() = default;

        string getType() override { return "/"; }

        double getValue() override { return childs.at(0)->getValue() / childs.at(1)->getValue(); }

        bool isFull() override { return childs.size() == 2; }
    };

    class OperatorPower : public OperatorNode {
    public:
        OperatorPower() = default;

        string getType() override { return "^"; }

        double getValue() override { return pow(childs.at(0)->getValue(), childs.at(1)->getValue()); }

        bool isFull() override { return childs.size() == 2; }
    };

private:
    static bool hasOpenLeftBracket(OperatorNode *node) {
        if (node->getType() == "(" && !node->isFull())
            return true;
        for (OperatorNode *tmp : node->childs)
            if (hasOpenLeftBracket(tmp))
                return true;
        return false;
    }

    void deleteIterator(OperatorNode *node) {
        for (OperatorNode *i : node->childs)
            deleteIterator(i);
        delete node;
    }

    OperatorNode *root = new OperatorLeftBracket();
public:
    SyntaxTree() = default;

    ~SyntaxTree() {
        if (root != nullptr) {
            deleteIterator(root);
        }
    }

    // For debug
    void printTree(OperatorNode *node, int depth) {
        for (auto i = 0; i < depth; i++)
            cout << "  ";
        cout << node->getType();
        if (node->getType() == "value")
            cout << ", " << node->getValue();
        cout << endl;
        for (OperatorNode *t : node->childs)
            printTree(t, depth + 1);
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
                    try {
                        root->addChild(new OperatorRightBracket);
                        cout << " = " << root->getValue() << endl;
                        return true;
                    } catch (out_of_range &e) {
                        cout << "syntax error" << endl;
                        return true;
                    } catch (runtime_error &e) {
                        cout << e.what() << endl;
                        return true;
                    }
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
    auto *s = new SyntaxTree();
    cout << "> ";
    while (s->parseFromStdIn()) {
        cout << "> ";
        delete s;
        s = new SyntaxTree();
    }
    delete s;
    return 0;
}