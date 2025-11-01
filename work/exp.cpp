// 引入头文件
#include <iostream>  // IO流
#include <map>       // 优先级设置
#include <vector>    // 存储变元
#include <set>       // 变元去重
#include <stack>     // 表达式计算
#include <string>    // 字符串处理
#include <algorithm> // 字符串操作
// #include <cmath>    // 幂计算 （可用位运算替代）

using namespace std;

// 初始类的定义
class setInitialParams
{
private:
    string formula;                 // 命题公式字符串
    vector<char> variables;         // 命题变元
    map<char, bool> variableValues; // 命题变元的值,键查询方式

    // 设置运算符优先级
    map<char, int> operatorPrecedence =
        {
            {'!', 5}, // 非
            {'&', 4}, // 合取
            {'|', 3}, // 析取
            {'-', 2}, // 蕴含
            {'=', 1}  // 等价
    };

    // 提取命题变元
    void extractVariables()
    {
        variables.clear(); // 清空之前的变元
        set<char> varSet;  // 使用set去重

        for (char ch : formula)
        {
            if (isalpha(ch) && islower(ch))
            {
                varSet.insert(ch); // 小写字母作为变元
            }
        }

        variables.assign(varSet.begin(), varSet.end()); // 将set转换为vector,此时varset是无序的
        sort(variables.begin(), variables.end());       // 对变元进行排序
    }

    // 设置命题变元的值（真或假）
    void setVariableValues(int row)
    {
        int var_nums = variables.size();
        for (int i = 0; i < var_nums; ++i)
        {
            // 使用位运算设置变元值
            variableValues[variables[i]] = (row >> (var_nums - i - 1)) & 1;
        }
    }

    // 中缀表达式转后缀表达式（字符串）
    string infixToPostfix(const string &infix)
    {
        string postfix = ""; // 初始化后缀表达式
        stack<char> opStack; // 定义运算符栈
        for (char c : infix)
        {
            if (c == ' ')
                continue; // 跳过空格
            if (isalpha(c) && islower(c))
                postfix += c; // 命题变元直接加入后缀表达式
            else if (c == '(')
            {
                opStack.push(c); // 左括号入栈
            }
            else if (c == ')')
            {
                if (opStack.empty())
                {
                    cerr << "Error: 运算符栈为空，无法进行匹配" << endl;
                    return "";
                }
                while (!opStack.empty() && opStack.top() != '(')
                {
                    postfix += opStack.top(); // 弹出栈顶运算符加入后缀表达式
                    opStack.pop();            // 弹出栈顶运算符
                }
                opStack.pop(); // 弹出左括号，防止阻塞
            }
            // 处理当前运算符和栈顶运算符的优先级
            else if (operatorPrecedence.count(c)) // 检查当前运算符是否为有效运算符
            {
                while (!opStack.empty() && opStack.top() != '(' && operatorPrecedence[opStack.top()] >= operatorPrecedence[c])
                {
                    postfix += opStack.top(); // 弹出栈顶运算符加入后缀表达式
                    opStack.pop();            // 弹出栈顶运算符
                }
                opStack.push(c); // 当前运算符入栈
            }
        }

        // 处理完中缀表达式后，清空运算符栈
        while (!opStack.empty())
        {
            postfix += opStack.top(); // 弹出栈顶运算符加入后缀表达式
            opStack.pop();            // 弹出栈顶运算符
        }

        return postfix; // 返回后缀表达式
    }

    // 计算后缀表达式（字符串）的值（bool值）
    bool calculatePostfixValue(const string &postfix)
    {
        stack<bool> valStack; // 定义值栈

        bool result = false; // 初始化计算结果值

        for (char c : postfix)
        {
            if (isalpha(c) && islower(c))
                valStack.push(variableValues[c]); // 将变元bool值压入值栈
            else
            {
                if (c == '!')
                {
                    bool operand = valStack.top(); // 获取要取反的变元bool值 / 操作数
                    valStack.pop();                // 弹出栈顶元素
                    valStack.push(!operand);       // 取反后压入值栈
                }
                else
                {
                    bool rightOperand = valStack.top();
                    valStack.pop(); // 获取右操作数，并压出栈
                    bool leftOperand = valStack.top();
                    valStack.pop(); // 获取左操作数，并压出栈

                    switch (c)
                    {
                    // 计算合取
                    case '&':
                        result = leftOperand && rightOperand;
                        valStack.push(result);
                        break;
                    // 计算析取
                    case '|':
                        result = leftOperand || rightOperand;
                        valStack.push(result);
                        break;
                    // 计算蕴含
                    case '-':
                        result = (!leftOperand) || rightOperand;
                        valStack.push(result);
                        break;
                    // 计算等值
                    case '=':
                        result = (leftOperand == rightOperand);
                        valStack.push(result);
                        break;
                    }
                }
            }
        }

        return valStack.top(); // 返回计算结果
    }

    // 检查括号匹配
    bool checkParenthesesMatching()
    {
        stack<char> parenStack; // 定义括号栈
        for (char c : formula)
        {
            if (c == '(')
                parenStack.push(c);
            else if (c == ')')
            {
                if (parenStack.empty())
                    return false;
                parenStack.pop();
            }
        }
        return parenStack.empty();
    }

public:
    setInitialParams(const string &formula) : formula(formula)
    {
        extractVariables();
        // 构造函数
    }
    ~setInitialParams()
    {
        // 析构函数
    }

    // 生成真值表
    void generateTruthTable()
    {
        // 检查括号匹配
        if (!checkParenthesesMatching())
        {
            cerr << "Error：括号不匹配！" << endl;
            return;
        }

        size_t var_nums = variables.size();
        size_t rows = 1 << var_nums;              // 2的var_nums次方
        string postfix = infixToPostfix(formula); // 中缀转后缀表达式

        cout << "The truthTable is as follows:" << endl;

        // 打印表头
        for (char var : variables)
        {
            cout << var << "\t";
        }
        cout << formula << endl;
        cout << string(8 * (var_nums + 1), '-') << endl;

        vector<vector<bool>> truthTable;

        for (size_t i = 0; i < rows; ++i)
        {
            // 设置变元值
            setVariableValues(i);

            // 输出当前行的变元值
            for (char var : variables)
            {
                cout << variableValues[var] << "\t";
            }

            // 输出当前行公式的真值
            bool result = calculatePostfixValue(postfix); // 计算后缀表达式值
            cout << result << endl;

            // 存储当前行的真值表
            vector<bool> rowValues;
            for (char var : variables)
            {
                rowValues.push_back(variableValues[var]);
            }
            rowValues.push_back(result);
            truthTable.push_back(rowValues);
        }

        // 判断公式类型
        determineFormulaType(truthTable);

        // 求主范式
        findPrincipalNormalFormula(truthTable);
    }

    // 判断命题公式类型
    void determineFormulaType(const vector<vector<bool>> &truthTable)
    {
        bool allTrue = true;
        bool allFalse = true;

        for (const auto &row : truthTable)
        {
            bool result = row.back();
            if (result)
                allFalse = false;
            else
                allTrue = false;
        }

        cout << "\n公式类型: ";
        if (allTrue)
        {
            cout << "永真式 (重言式)" << endl;
        }
        else if (allFalse)
        {
            cout << "永假式 (矛盾式)" << endl;
        }
        else
        {
            cout << "可满足式" << endl;
        }
    }

    // 求主范式
    void findPrincipalNormalFormula(const vector<vector<bool>> &truthTable)
    {
        vector<string> minterms; // 极小项（主析取范式）
        vector<string> maxterms; // 极大项（主合取范式）

        int n = variables.size();

        for (int i = 0; i < truthTable.size(); i++)
        {
            const auto &row = truthTable[i];
            bool result = row.back();

            if (result)
            { // 真值为真，对应极小项
                string minterm = "(";
                for (int j = 0; j < n; j++)
                {
                    if (j > 0)
                        minterm += " & ";
                    minterm += row[j] ? string(1, variables[j]) : ("!" + string(1, variables[j]));
                }
                minterm += ")";
                minterms.push_back(minterm);
            }
            else
            { // 真值为假，对应极大项
                string maxterm = "(";
                for (int j = 0; j < n; j++)
                {
                    if (j > 0)
                        maxterm += " | ";
                    maxterm += row[j] ? ("!" + string(1, variables[j])) : string(1, variables[j]);
                }
                maxterm += ")";
                maxterms.push_back(maxterm);
            }
        }

        // 输出主析取范式
        cout << "\n主析取范式 (极小项的析取):" << endl;
        if (minterms.empty())
        {
            cout << "不存在（永假式）" << endl;
        }
        else
        {
            for (size_t i = 0; i < minterms.size(); i++)
            {
                cout << minterms[i];
                if (i < minterms.size() - 1)
                    cout << " | ";
            }
            cout << endl;
        }

        // 输出主合取范式
        cout << "\n主合取范式 (极大项的合取):" << endl;
        if (maxterms.empty())
        {
            cout << "不存在（永真式）" << endl;
        }
        else
        {
            for (size_t i = 0; i < maxterms.size(); i++)
            {
                cout << maxterms[i];
                if (i < maxterms.size() - 1)
                    cout << " & ";
            }
            cout << endl;
        }
    }
};

// main主函数
int main()
{
    // 介绍界面
    cout << "=== 命题公式真值表及运算系统 ===" << endl;
    cout << "运算符说明: " << endl;
    cout << "! : 非        & : 合取        | : 析取        - : 蕴含        = : 等价" << endl;
    cout << "示例: (p - q) | (q & !r)" << endl;
    cout << "输入 'quit' 退出程序" << endl;

    // 获得输入
    while (true)
    {
        // 输入命题公式
        cout << "请输入命题公式: ";
        string formula;
        getline(cin, formula);

        // 输入quit退出程序
        if (formula == "quit")
            break;

        // 输入为空继续输入
        if (formula.empty())
        {
            cerr << "Error: 输入不能为空，请重新输入。" << endl;
            continue;
        }

        // 生成真值表
        try
        {
            setInitialParams sip(formula);
            sip.generateTruthTable();
        }
        catch (const exception &e)
        {
            cout << "错误: " << e.what() << endl;
        }
    }
    return 0;
}