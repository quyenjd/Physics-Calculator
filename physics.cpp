/*
   Copyright 2018 @quyenjd (Quyen Dinh)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <bits/stdc++.h>
#include <conio.h>
#include "exprtk.hpp"

#define __pc_getch() cerr<<endl<<"Press any key to continue... ";_getch()
#define clrscr system("cls")

using namespace std;

/* === CONSTANTS === */

const string __pc_PCName    = "Physics Calculator";
const string __pc_PCVersion = "1.0a";
const string __pc_PCAuthor  = "quyenjd";

const string __pc_Functions_fName = "physics_Functions.pc";
const string __pc_Variables_fName = "physics_Variables.pc";
const string __pc_Constants_fName = "physics_Constants.pc";
const string __pc_Formulas_fName  = "physics_Formulas.pc";
const string __pc_Log_fName       = "physics_Log.pc";

string StrFormat (const char* __pc_Format...)
{
    stringstream Result;
    va_list Args;
    va_start(Args, __pc_Format);
    while (*__pc_Format != '\0')
    {
        char c = *__pc_Format;
        if (c == '%')
        {
            ++__pc_Format;
            if (*__pc_Format == 'd')
                Result << va_arg(Args, int);
            else
            if (*__pc_Format == 'c')
                Result << char(va_arg(Args, int));
            else
            if (*__pc_Format == 's')
                Result << va_arg(Args, string);
            else
            if (*__pc_Format == 'f')
                Result << va_arg(Args, double);
            else
                Result << c << *__pc_Format;
        }
        else Result << c;
        ++__pc_Format;
    }
    return Result.str();
}

/* === DECLARATIONS === */

struct Stats
{
    int __pc_FunctionFound;
    int __pc_FunctionFoundFailed;
    int __pc_FunctionAdded;

    int __pc_VariableFound;
    int __pc_VariableFoundFailed;
    int __pc_VariableAdded;

    int __pc_ConstantFound;
    int __pc_ConstantFoundFailed;
    int __pc_ConstantAdded;

    int __pc_FormulaFound;
    int __pc_FormulaFoundFailed;
    int __pc_FormulaAdded;
    int __pc_FormulaUsed;

    int __pc_ExpressionSolved;

    bool __pc_ShowError = 1;

    vector<string> __pc_Logs;
    void Log (const string& __pc_LogLine)
    {
        __pc_Logs.push_back(__pc_LogLine);
        if (__pc_ShowError)
            cerr << endl << StrFormat("===[LOGGER].ShowError()=== reported: %s", __pc_LogLine) << endl;
    }

    void Export (bool toerr = 1)
    {
        cerr << "Writing logs... (" << __pc_Logs.size() << " line" << (__pc_Logs.size() > 1 ? "s" : "\0") << ")" << endl;
        if (toerr)
            cerr << "===" << endl;

        ofstream Log_FS (__pc_Log_fName);
        for (string LogLine: __pc_Logs)
        {
            if (toerr)
                cerr << LogLine << endl;
            Log_FS << LogLine << endl;
        }

        cerr << endl;
        cerr << "Writing stats..." << endl;
        cerr << "__pc_ConstantFound       =" << __pc_ConstantFound       << endl;
        cerr << "__pc_ConstantFoundFailed =" << __pc_ConstantFoundFailed << endl;
        cerr << "__pc_ConstantAdded       =" << __pc_ConstantAdded       << endl;
        cerr << endl;
        cerr << "__pc_ExpressionSolved    =" << __pc_ExpressionSolved    << endl;
        cerr << endl;
        cerr << "__pc_FormulaFound        =" << __pc_FormulaFound        << endl;
        cerr << "__pc_FormulaFoundFailed  =" << __pc_FormulaFoundFailed  << endl;
        cerr << "__pc_FormulaAdded        =" << __pc_FormulaAdded        << endl;
        cerr << "__pc_FormulaUsed         =" << __pc_FormulaUsed         << endl;
        cerr << endl;
        cerr << "__pc_FunctionFound       =" << __pc_FunctionFound       << endl;
        cerr << "__pc_FunctionFoundFailed =" << __pc_FunctionFoundFailed << endl;
        cerr << "__pc_FunctionAdded       =" << __pc_FunctionAdded       << endl;
        cerr << endl;
        cerr << "__pc_VariableFound       =" << __pc_VariableFound       << endl;
        cerr << "__pc_VariableFoundFailed =" << __pc_VariableFoundFailed << endl;
        cerr << "__pc_VariableAdded       =" << __pc_VariableAdded       << endl;
        cerr << endl;

        Log_FS.close();
    }
};

set<string> __pc_Functions;

struct Variable
{
    string VarName;
    string Description;

    Variable (string __VarName = "", string __Description = ""):
        VarName(__VarName), Description(__Description) {}
};
unordered_map<string, Variable> __pc_Variables;

struct Const
{
    string ConName;
    double Value;
    string Description;

    Const (string __ConName = "", double __Value = 0.0, string __Description = ""):
        ConName(__ConName), Value(__Value), Description(__Description) {}
};
unordered_map<string, Const> __pc_Constants;

struct Formula
{
    string Expression;
    string Description;
    set<string> Tokens;

    Formula (string __Expression = "", string __Description = ""):
        Expression(__Expression), Description(__Description)
    {
        string __Tmp;
        for (size_t index_i = 0; index_i < Expression.length(); ++index_i)
            if (Expression[index_i] != ' ')
                __Tmp += Expression[index_i];
        Expression = __Tmp;
    }

    string Root        = "unknown";
    string Progression = "empty";

    bool FetchToken (Stats& __pc_Ends)
    {
        Tokens.clear();

        unsigned int __EqualSigns = 0;
        string __Working;
        for (size_t index_i = 0; index_i < Expression.length(); ++index_i)
            if (Expression[index_i] == '=')
            {
                ++__EqualSigns;
                __Working += ":=";
            }
            else
                __Working += Expression[index_i];

        if (__EqualSigns > 1 || __EqualSigns == 0)
        {
            __pc_Ends.Log(StrFormat("[Formula.FetchToken()]   Compilation error[INVALID_EQUALSIGNS]! Invalid Expression:%s.", Expression));
            return false;
        }

        for (size_t index_i = 0; index_i < Expression.length(); ++index_i)
            if (isalpha(Expression[index_i]))
            {
                if (index_i && isdigit(Expression[index_i - 1]))
                {
                    __pc_Ends.Log(StrFormat("[Formula.FetchToken()]   Compilation error[INVALID_TOKEN]! Invalid Expression:%s.", Expression));
                    return false;
                }

                string __Tmp;
                __Tmp += Expression[index_i++];
                while (index_i < Expression.length() && isalnum(Expression[index_i]))
                    __Tmp += Expression[index_i++];
                --index_i;

                if (__pc_Variables.find(__Tmp) != __pc_Variables.end())
                {
                    Tokens.insert(__Tmp);
                    continue;
                }

                string __abc;
                for (char x: __Tmp)
                    __abc += tolower(x);

                if (__abc != "pi" && __abc != "e" &&
                    __pc_Constants.find(__Tmp) == __pc_Constants.end() && __pc_Functions.find(__Tmp) == __pc_Functions.end())
                {
                    __pc_Ends.Log(StrFormat("[Formula.FetchToken()]   Compilation error[UNKNOWN_TOKEN]! Invalid Expression:%s (Token:%s).", Expression, __Tmp));
                    return false;
                }
            }

        exprtk::symbol_table<double> symbol_table;
        exprtk::expression<double>   expression;
        exprtk::parser<double>       parser;

        vector<double> __Savers;

        for (string __Token: Tokens)
        {
            __Savers.push_back(10.0);
            symbol_table.add_variable(__Token, __Savers[__Savers.size() - 1]);
        }

        symbol_table.add_constants();
        expression.register_symbol_table(symbol_table);

        if (!parser.compile(__Working, expression))
        {
            __pc_Ends.Log(StrFormat("[Formula.FetchToken()/exprtk::parser.compile()]   Compilation error! Invalid Expression:%s.", Expression));
            return false;
        }

        Root.clear();
        Progression.clear();
        for (size_t index_i = 0; index_i < Expression.length(); ++index_i)
            if (Expression[index_i] == '=')
            {
                Progression = Root;
                Root.clear();
            }
            else
                Root += Expression[index_i];
        swap(Root, Progression);

        return true;
    }
};
unordered_map<string, vector<Formula> > __pc_Formulas;

typedef unordered_map<string, vector<Formula> > ::iterator MapFormula_Iterator;

/* Preparation */
void LoadFunctions     (set<string>&                             __pc_Functions,                                          Stats& __pc_Ends);
void ScanVariables     (unordered_map<string, Variable>&         __pc_MapVariable,                                        Stats& __pc_Ends);
void ScanConstants     (unordered_map<string, Const>&            __pc_MapConst,                                           Stats& __pc_Ends);
void ScanFormulas      (unordered_map<string, vector<Formula> >& __pc_MapFormulas,                                        Stats& __pc_Ends);

/* Queries */
void FindFormula       (const set<string>& __pc_Arguments, vector<Formula>& __pc_VectorFormula, const string& __pc_Root,  Stats& __pc_Ends);
bool CheckNameVaild    (const string& __pc_Name, const string& __pc_Type, bool __pc_Log,                                  Stats& __pc_Ends);

/* Developers */
bool AddFunction       (const string& __pc_NewFunction,                                                                   Stats& __pc_Ends);
bool AddVariable       (const string& __pc_NewVariable, const string& __pc_Description,                                   Stats& __pc_Ends);
bool AddConstant       (const string& __pc_NewConstant, const double& __pc_Value, const string& __pc_Description,         Stats& __pc_Ends);
bool AddFormula        (const string& __pc_NewFormula,  const string& __pc_Description,                                   Stats& __pc_Ends);

/* Solvers */
void BrainStorm        (const string& __pc_Expression, set<string> __pc_Forbid, int depth,                                Stats& __pc_Ends);
void ParseExpression   (const string& __pc_Expression, unordered_map<string, double> __pc_UserInput, double& __pc_Answer, Stats& __pc_Ends);
void RemoveDuplicates  (unordered_map<string, vector<Formula> >& __pc_MapFormulas, int& __pc_DuplicateRemoved);
void PhysicsCalculator (Stats& __pc_Ends);

/* Endings */
void SaveFunctions     (const set<string>&                             __pc_Functions);
void SaveVariables     (const unordered_map<string, Variable>&         __pc_MapVariable);
void SaveConstants     (const unordered_map<string, Const>&            __pc_MapConst);
void SaveFormulas      (const unordered_map<string, vector<Formula> >& __pc_MapFormulas);

/* Stuffs */
void Greetings ();
void Report    (Stats& __pc_Ends);

/* === IMPLEMENTATION === */

void LoadFunctions (set<string>& __pc_Functions, Stats& __pc_Ends)
{
    __pc_Functions.clear();

    ifstream Function_FS (__pc_Functions_fName);
    string Str;
    while (getline(Function_FS, Str))
    {
        istringstream Str_SS (Str);
        string __Func;
        Str_SS >> __Func;

        if (__pc_Functions.find(__Func) != __pc_Functions.end())
            ++__pc_Ends.__pc_FunctionFoundFailed;
        else
            __pc_Functions.insert(__Func);
    }
    Function_FS.close();
}

void ScanVariables (unordered_map<string, Variable>& __pc_MapVariable, Stats& __pc_Ends)
{
    __pc_MapVariable.clear();

    ifstream Variable_FS (__pc_Variables_fName);
    string Str;
    while (getline(Variable_FS, Str))
    {
        istringstream Str_SS (Str);
        string __VarName, __Description, __Tmp;
        Str_SS >> __VarName;

        string __abc;
        for (char x: __VarName)
            __abc += tolower(x);
        if (__abc == "pi" || __abc == "e")
        {
            __pc_Ends.Log(StrFormat("[ScanVariables]   Forbidden variable name found VarName:%s", __VarName));
            ++__pc_Ends.__pc_VariableFoundFailed;
            continue;
        }

        while (Str_SS >> __Tmp)
            __Description += __Tmp + ' ';
        __Description.pop_back();

        if (__pc_MapVariable.find(__VarName) != __pc_MapVariable.end())
        {
            __pc_Ends.Log(StrFormat("[ScanVariables]   Conflict detected on Variable:%s.", __VarName));
            ++__pc_Ends.__pc_VariableFoundFailed;
            continue;
        }

        __pc_MapVariable[__VarName] = Variable(__VarName, __Description);
        ++__pc_Ends.__pc_VariableFound;
    }
    Variable_FS.close();
}

void ScanConstants (unordered_map<string, Const>& __pc_MapConst, Stats& __pc_Ends)
{
    __pc_MapConst.clear();

    ifstream Constant_FS (__pc_Constants_fName);
    string Str;
    while (getline(Constant_FS, Str))
    {
        istringstream Str_SS (Str);
        string __ConName, __Description, __Tmp;
        double __Value;
        Str_SS >> __ConName >> __Value;

        string __abc;
        for (char x: __ConName)
            __abc += tolower(x);
        if (__abc == "pi" || __abc == "e")
        {
            __pc_Ends.Log(StrFormat("[ScanConstants]   Forbidden constant name found ConName:%s", __ConName));
            ++__pc_Ends.__pc_ConstantFoundFailed;
            continue;
        }

        while (Str_SS >> __Tmp)
            __Description += __Tmp + ' ';
        __Description.pop_back();

        if (__pc_MapConst.find(__ConName) != __pc_MapConst.end())
        {
            __pc_Ends.Log(StrFormat("[ScanConstants]   Conflict detected on Constant:%s.", __ConName));
            ++__pc_Ends.__pc_ConstantFoundFailed;
            continue;
        }

        __pc_MapConst[__ConName] = Const(__ConName, __Value, __Description);
        ++__pc_Ends.__pc_ConstantFound;
    }
    Constant_FS.close();
}

void ScanFormulas (unordered_map<string, vector<Formula> >& __pc_MapFormulas, Stats& __pc_Ends)
{
    __pc_MapFormulas.clear();

    ifstream Formula_FS (__pc_Formulas_fName);
    string Str;
    while (getline(Formula_FS, Str))
    {
        istringstream Str_SS (Str);
        string __Formula, __Description, __Tmp;

        Str_SS >> __Formula;
        Formula __Working(__Formula);

        if (!__Working.FetchToken(__pc_Ends))
        {
            ++__pc_Ends.__pc_FormulaFoundFailed;
            continue;
        }

        while (Str_SS >> __Tmp)
            __Description += __Tmp + ' ';
        __Description.pop_back();

        __Working.Description = __Description;

        __pc_MapFormulas[__Working.Root].push_back(__Working);
        ++__pc_Ends.__pc_FormulaFound;
    }
    Formula_FS.close();
}

void FindFormula (const set<string>& __pc_Arguments, vector<Formula>& __pc_VectorFormula, const string& __pc_Root, Stats& __pc_Ends)
{
    for (MapFormula_Iterator iter = __pc_Formulas.begin(); iter != __pc_Formulas.end(); ++iter)
    {
        if (iter->first != __pc_Root)
            continue;

        vector<Formula> __Working = iter->second;

        for (Formula __Tmp: __Working)
        {
            if (__Tmp.Root != __pc_Root)
                continue;

            unsigned int __Count = 0;

            for (string __Arg: __pc_Arguments)
                if (__Tmp.Tokens.find(__Arg) != __Tmp.Tokens.end())
                    ++__Count;

            if (__Count == __pc_Arguments.size())
            {
                __pc_VectorFormula.push_back(__Tmp);
                ++__pc_Ends.__pc_FormulaUsed;
            }
        }
    }
}

bool CheckNameVaild (const string& __pc_Name, const string& __pc_Type, bool __pc_Log, Stats& __pc_Ends)
{
    if (__pc_Name.empty())
        return false;

    if (!isalpha(__pc_Name[0]))
        return false;

    for (char __Character: __pc_Name)
        if (!isalnum(__Character))
            return false;

    if (__pc_Functions.find(__pc_Name) != __pc_Functions.end())
    {
        if (__pc_Log)
            __pc_Ends.Log(StrFormat("[CheckNameVaild]   Conflict detected on %s:%s [NAME_WAS_USED_BY_SOME_FUNCTION].", __pc_Type, __pc_Name));
        return false;
    }

    if (__pc_Constants.find(__pc_Name) != __pc_Constants.end())
    {
        if (__pc_Log)
            __pc_Ends.Log(StrFormat("[CheckNameVaild]   Conflict detected on %s:%s [NAME_WAS_USED_BY_SOME_CONSTANT].", __pc_Type, __pc_Name));
        return false;
    }

    if (__pc_Variables.find(__pc_Name) != __pc_Variables.end())
    {
        if (__pc_Log)
            __pc_Ends.Log(StrFormat("[CheckNameVaild]   Conflict detected on %s:%s [NAME_WAS_USED_BY_SOME_VARIABLE].", __pc_Type, __pc_Name));
        return false;
    }

    return true;
}

bool AddFunction (const string& __pc_NewFunction, Stats& __pc_Ends)
{
    if (!CheckNameVaild(__pc_NewFunction, "Function", 1, __pc_Ends))
        return false;

    __pc_Functions.insert(__pc_NewFunction);
    ++__pc_Ends.__pc_FunctionAdded;
    return true;
}

bool AddVariable (const string& __pc_NewVariable, const string& __pc_Description, Stats& __pc_Ends)
{
    if (!CheckNameVaild(__pc_NewVariable, "Variable", 1, __pc_Ends))
        return false;

    string __abc;
    for (char x: __pc_NewVariable)
        __abc += tolower(x);
    if (__abc == "pi" || __abc == "e")
    {
        __pc_Ends.Log(StrFormat("[AddVariable]   Forbidden variable name found NewVariable:%s", __pc_NewVariable));
        return false;
    }

    __pc_Variables[__pc_NewVariable] = Variable(__pc_NewVariable, __pc_Description);
    ++__pc_Ends.__pc_VariableAdded;
    return true;
}

bool AddConstant (const string& __pc_NewConstant, const double& __pc_Value, const string& __pc_Description, Stats& __pc_Ends)
{
    if (!CheckNameVaild(__pc_NewConstant, "Constant", 1, __pc_Ends))
        return false;

    string __abc;
    for (char x: __pc_NewConstant)
        __abc += tolower(x);
    if (__abc == "pi" || __abc == "e")
    {
        __pc_Ends.Log(StrFormat("[AddConstant]   Forbidden constant name found NewConstant:%s", __pc_NewConstant));
        return false;
    }

    __pc_Constants[__pc_NewConstant] = Const(__pc_NewConstant, __pc_Value, __pc_Description);
    ++__pc_Ends.__pc_ConstantAdded;
    return true;
}

bool AddFormula (const string& __pc_NewFormula, const string& __pc_Description, Stats& __pc_Ends)
{
    Formula __Working(__pc_NewFormula, __pc_Description);
    if (!__Working.FetchToken(__pc_Ends))
    {
        __pc_Ends.Log(StrFormat("[AddFormula]   Invalid Formula:%s (Root:%s). Rejected.", __pc_NewFormula, __Working.Root));
        return false;
    }

    __pc_Formulas[__Working.Root].push_back(__Working);
    ++__pc_Ends.__pc_FormulaAdded;
    return true;
}

void BrainStorm (const string& __pc_Expression, set<string> __pc_Forbid, int depth, Stats& __pc_Ends)
{
    Formula __Working(__pc_Expression, "FetchToken() is used in BrainStorm.");
    __Working.FetchToken(__pc_Ends);
    for (string __Token: __Working.Tokens)
    {
        if (__Token == __Working.Root || __pc_Forbid.find(__Token) != __pc_Forbid.end())
            continue;

        for (Formula __Tmp: __pc_Formulas[__Token])
        {
            bool __Chosen = 0;
            for (string __Token2: __Tmp.Tokens)
                if (__Working.Tokens.find(__Token2) == __Working.Tokens.end() && __pc_Forbid.find(__Token2) == __pc_Forbid.end())
                {
                    __Chosen = 1;
                    break;
                }

            if (__Chosen)
            {
                string __Entry = __pc_Expression;
                regex __Regex(StrFormat("(?=[^:alnum:]|$)(%s)(?=[^:alnum:]|$)", __Token));
                __Entry = regex_replace(__Entry, __Regex, StrFormat("(%s)", __Tmp.Progression));

                __pc_Forbid.insert(__Tmp.Root);

                cerr << StrFormat("[BrainStorm]   Expression:%s is generated! (depth=%d, chosen:%s)", __Entry, depth, __Tmp.Expression) << endl;
                AddFormula(__Entry, StrFormat("Generated by BrainStorm from Formula:%s (depth=%d)", __pc_Expression, depth), __pc_Ends);
                BrainStorm(__Entry, __pc_Forbid, depth + 1, __pc_Ends);
            }
        }
    }
}

void ParseExpression (const string& __pc_Expression, unordered_map<string, double> __pc_UserInput, double& __pc_Answer, Stats& __pc_Ends)
{
    __pc_Answer = 0;
    Formula __Expression(__pc_Expression);

    if (!__Expression.FetchToken(__pc_Ends))
    {
        __pc_Ends.Log(StrFormat("[ParseExpression]   Error while fetching token on Formula:%s.", __pc_Expression));
        return;
    }

    for (string __Token: __Expression.Tokens)
        if (__Token != __Expression.Root && __pc_UserInput.find(__Token) == __pc_UserInput.end())
        {
            __pc_Ends.Log(StrFormat("[ParseExpression]   Formula's token list and User's are unsynchronized. Error on Token:%s in Formula:%s.",
                                    __Token, __pc_Expression));
            return;
        }

    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double>   expression;
    exprtk::parser<double>       parser;

    vector<double> __Savers;

    for (string __Token: __Expression.Tokens)
    {
        __Savers.push_back(__pc_UserInput[__Token]);
        symbol_table.add_variable(__Token, __Savers[__Savers.size() - 1]);
    }

    symbol_table.add_constants();
    expression.register_symbol_table(symbol_table);
    parser.compile(__Expression.Progression, expression);

    __pc_Answer = expression.value();

    ++__pc_Ends.__pc_ExpressionSolved;
}

void RemoveDuplicates (unordered_map<string, vector<Formula> >& __pc_MapFormula, int& __pc_DuplicateRemoved)
{
    vector<set<string> > __Savers;
    unordered_map<string, vector<Formula> > __Result;
    __pc_DuplicateRemoved = 0;

    for (MapFormula_Iterator iter = __pc_MapFormula.begin(); iter != __pc_MapFormula.end(); ++iter)
    {
        __Savers.clear();
        __Result[iter->first];

        for (Formula __Working: iter->second)
        {
            bool __ToBeAdded = 1;

            for (set<string> __Tmp: __Savers)
            {
                if (__Working.Tokens.size() != __Tmp.size())
                    continue;

                bool __Same = 1;
                for (string __Token: __Tmp)
                    if (__Working.Tokens.find(__Token) == __Working.Tokens.end())
                    {
                        __Same = 0;
                        break;
                    }

                if (__Same)
                {
                    __ToBeAdded = 0;
                    break;
                }
            }

            if (__ToBeAdded)
            {
                __Result[iter->first].push_back(__Working);
                __Savers.push_back(__Working.Tokens);
            }
            else
                ++__pc_DuplicateRemoved;
        }
    }

    __pc_MapFormula = __Result;
}

void PhysicsCalculator (Stats& __pc_Ends)
{
    cerr << "[SCANNER]   Scanning functions..." << endl;
    LoadFunctions(__pc_Functions, __pc_Ends);
    cerr << "[SCANNER]   Scanning variables..." << endl;
    ScanVariables(__pc_Variables, __pc_Ends);
    cerr << "[SCANNER]   Scanning constants..." << endl;
    ScanConstants(__pc_Constants, __pc_Ends);
    cerr << "[SCANNER]   Scanning formulas..."  << endl;
    ScanFormulas (__pc_Formulas , __pc_Ends);

    while (1)
    {
        clrscr;
        Greetings();

        string __Cmd;
        cerr << "Your command:   ";
        getline(cin, __Cmd);

        istringstream Str_SS (__Cmd);

        string __Pointer;
        Str_SS >> __Pointer;

        if (__Pointer == "const")
        {
                // CONST COMMAND

                string __NewConstant, __Description, __Tmp;
                double __Value;
                Str_SS >> __NewConstant >> __Value;
                while (Str_SS >> __Tmp)
                    __Description += __Tmp + ' ';

                if (__Description.length())
                    __Description.pop_back();

                cerr << StrFormat("[FYI - New Constant]   Name=%s, Value=%f, Description=\"%s\".",
                                  __NewConstant, __Value, __Description) << endl;
                cerr << "Do you want to add this constant? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    cerr << "Adding ..." << endl;
                    bool __Result = AddConstant(__NewConstant, __Value, __Description, __pc_Ends);

                    if (__Result)
                        cerr << "New constant added!" << endl;
                    else
                        cerr << "Error while adding constant! Please try again..." << endl;
                    __pc_getch();
                }
        }

        if (__Pointer == "formula")
        {
                // FORMULA COMMAND

                string __NewFormula, __Description, __Tmp;
                Str_SS >> __NewFormula;
                while (Str_SS >> __Tmp)
                    __Description += __Tmp + ' ';

                if (__Description.length())
                    __Description.pop_back();

                cerr << StrFormat("[FYI - New Formula]   Expression:%s, Description=\"%s\".",
                                  __NewFormula, __Description) << endl;
                cerr << "Do you want to add this formula? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    cerr << "Adding ..." << endl;
                    bool __Result = AddFormula(__NewFormula, __Description, __pc_Ends);

                    if (__Result)
                    {
                        cerr << "New formula added!" << endl;
                        cerr << "Learning ..." << endl;

                        cerr << "[Brainstorm-Caller]   Copying data..." << endl;
                        vector<Formula> __Formulas;
                        for (pair<string, vector<Formula> > __Tmp: __pc_Formulas)
                            for (Formula __Tmp2: __Tmp.second)
                                __Formulas.push_back(__Tmp2);

                        for (size_t index_i = 0; index_i < __Formulas.size(); ++index_i)
                        {
                            int __SaverFormulaAdded = __pc_Ends.__pc_FormulaAdded;
                            BrainStorm(__Formulas[index_i].Expression, set<string>(), 0, __pc_Ends);
                            cerr << StrFormat("[Brainstorm-Caller]   Learning done from Formula:%s (found %d formulas).",
                                              __Formulas[index_i].Expression, __pc_Ends.__pc_FormulaAdded - __SaverFormulaAdded) << endl;
                        }

                        cerr << "Removing duplicates..." << endl;
                        int __Counter = 0;
                        RemoveDuplicates(__pc_Formulas, __Counter);
                        cerr << "[RemoveDuplicates]   " << __Counter << " duplicate" << (__Counter > 1 ? "s" : "") << " removed." << endl;

                        cerr << endl;
                        cerr << "Complete learning." << endl;
                    }
                    else
                        cerr << "Error while adding formula! Please try again..." << endl;
                    __pc_getch();
                }
        }


        if (__Pointer == "func")
        {
                // FUNC COMMAND

                string __NewFunction;
                Str_SS >> __NewFunction;

                string __Tmp;
                cerr << StrFormat("[FYI - New Function]   Name=%s.", __NewFunction) << endl;
                cerr << "Do you want to add this function? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    bool __Result = AddFunction(__NewFunction, __pc_Ends);
                    if (__Result)
                        cerr << "New function added!" << endl;
                    else
                        cerr << "Error while adding function! Please try again..." << endl;
                    __pc_getch();
                }
        }

        if (__Pointer == "solve")
        {
                // SOLVE COMMAND

                string __Args, __Root, __Tmp;

                set<string> __Arguments;
                unordered_map<string, double> __UserInput;
                while (Str_SS >> __Tmp)
                {
                    if (__Tmp[0] == '?')
                    {
                        __Tmp.erase(__Tmp.begin());
                        __Root = __Tmp;
                        break;
                    }

                    string __Var, __Value;
                    for (size_t index_i = 0; index_i < __Tmp.length(); ++index_i)
                        if (__Tmp[index_i] == '=')
                        {
                            __Value = __Var;
                            __Var = "";
                        }
                        else
                            __Var += __Tmp[index_i];
                    swap(__Var, __Value);

                    __Arguments.insert(__Var);
                    __UserInput[__Var] = stod(__Value);

                    __Args += StrFormat("%s=%s,", __Var, to_string(__UserInput[__Var]));
                }

                if (__Args.length())
                    __Args.pop_back();

                cerr << StrFormat("[FYI - Equation]   Args={%s}, Root=%s.", __Args, __Root) << endl;
                cerr << "Do you want to find and solve formulas with these args? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    cerr << "Finding suitable formulas..." << endl;
                    vector<Formula> __SuitableExpressions;
                    FindFormula(__Arguments, __SuitableExpressions, __Root, __pc_Ends);

                    if (__SuitableExpressions.size())
                        cerr << StrFormat("%d formulas found.", __SuitableExpressions.size()) << endl << endl;
                    else
                        cerr << "No formulas found." << endl;

                    for (Formula __Working: __SuitableExpressions)
                    {
                        double __Answer = 0.0;
                        ParseExpression(__Working.Expression, __UserInput, __Answer, __pc_Ends);
                        cerr << StrFormat("[Solver]   %s, Answer=%f.", __Working.Expression, __Answer) << endl;
                    }

                    cerr << endl;
                    __pc_getch();
                }
        }

        if (__Pointer == "var")
        {
                // VAR COMMAND

                string __NewVariable, __Description, __Tmp;
                Str_SS >> __NewVariable;
                while (Str_SS >> __Tmp)
                    __Description += __Tmp + ' ';

                if (__Description.length())
                    __Description.pop_back();

                cerr << StrFormat("[FYI - New Variable]   Name=%s, Description=\"%s\".", __NewVariable, __Description) << endl;
                cerr << "Do you want to add this variable? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    cerr << "Adding ..." << endl;
                    bool __Result = AddVariable(__NewVariable, __Description, __pc_Ends);

                    if (__Result)
                        cerr << "New variable added!" << endl;
                    else
                        cerr << "Error while adding variable! Please try again..." << endl;
                    __pc_getch();
                }
        }

        if (__Pointer == "consts")
        {
                // CONSTS COMMAND

                cerr << "Listing all constants..." << endl;
                for (pair<string, Const> __Working: __pc_Constants)
                    cerr << StrFormat("(Key=%s) Name=%s, Value=%f, Description=\"%s\".",
                                      __Working.first, __Working.second.ConName, __Working.second.Value, __Working.second.Description) << endl;
                cerr << "___ End of list." << endl;
                __pc_getch();
        }

        if (__Pointer == "formulas")
        {
                // FORMULAS COMMAND

                cerr << "Listing all formulas..." << endl;
                for (pair<string, vector<Formula> > __Working: __pc_Formulas)
                    for (Formula __Tmp: __Working.second)
                        cerr << StrFormat("(Key=%s) Formula:%s, Description=\"%s\".",
                                          __Working.first, __Tmp.Expression, __Tmp.Description) << endl;
                cerr << "___ End of list." << endl;
                __pc_getch();
        }

        if (__Pointer == "funcs")
        {
                // FUNCS COMMAND

                cerr << "Listing all functions..." << endl;
                string __List;
                for (string __Working: __pc_Functions)
                    __List += __Working + ", ";

                if (__List.length())
                {
                    __List.pop_back();
                    __List.pop_back();
                }

                cerr << StrFormat("{ %s }", __List) << endl;
                __pc_getch();
        }

        if (__Pointer == "vars")
        {
                // VARS COMMAND

                cerr << "Listing all variables..." << endl;
                for (pair<string, Variable> __Working: __pc_Variables)
                    cerr << StrFormat("(Key=%s) Name=%s, Description=\"%s\".",
                                      __Working.first, __Working.second.VarName, __Working.second.Description) << endl;
                cerr << "___ End of list." << endl;
                __pc_getch();
        }

        if (__Pointer == "reload")
        {
                // RELOAD COMMAND

                string __Tmp;
                cerr << "Do you want to reload all data (from files)? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    cerr << "Reloading..." << endl;
                    LoadFunctions(__pc_Functions, __pc_Ends);
                    ScanConstants(__pc_Constants, __pc_Ends);
                    ScanVariables(__pc_Variables, __pc_Ends);
                    ScanFormulas (__pc_Formulas , __pc_Ends);
                }
        }

        if (__Pointer == "save")
        {
                // SAVE COMMAND

                string __Tmp;
                cerr << "Do you want to save all data (to files)? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                {
                    cerr << "[PRINTER]   Scanning functions..." << endl;
                    SaveFunctions(__pc_Functions);
                    cerr << "[PRINTER]   Scanning variables..." << endl;
                    SaveVariables(__pc_Variables);
                    cerr << "[PRINTER]   Scanning constants..." << endl;
                    SaveConstants(__pc_Constants);
                    cerr << "[PRINTER]   Scanning formulas..."  << endl;
                    SaveFormulas (__pc_Formulas );
                }
        }

        if (__Pointer == "quit")
        {
                // QUIT COMMAND

                string __Tmp;
                cerr << "Are you sure? (type \"yes\" for YES, NO otherwise)   ";
                cin >> __Tmp;

                if (__Tmp == "yes")
                    return;
        }
    }
}

void SaveFunctions (const set<string>& __pc_Functions)
{
    ofstream Function_FS (__pc_Functions_fName);
    for (string __Working: __pc_Functions)
        Function_FS << __Working << endl;
    Function_FS.close();
}

void SaveVariables (const unordered_map<string, Variable>& __pc_MapVariable)
{
    ofstream Variable_FS (__pc_Variables_fName);
    for (pair<string, Variable> __Working: __pc_MapVariable)
        Variable_FS << __Working.second.VarName << " " << __Working.second.Description << endl;
    Variable_FS.close();
}

void SaveConstants (const unordered_map<string, Const>& __pc_MapConst)
{
    ofstream Constant_FS (__pc_Constants_fName);
    for (pair<string, Const> __Working: __pc_MapConst)
        Constant_FS << __Working.second.ConName << " " << fixed << setprecision(10) << __Working.second.Value << " " << __Working.second.Description << endl;
    Constant_FS.close();
}

void SaveFormulas (const unordered_map<string, vector<Formula> >& __pc_MapFormula)
{
    ofstream Formula_FS (__pc_Formulas_fName);
    for (pair<string, vector<Formula> > __Working: __pc_MapFormula)
        for (Formula __Tmp: __Working.second)
            Formula_FS << __Tmp.Expression << " " << __Tmp.Description << endl;
    Formula_FS.close();
}

void Greetings ()
{
    cerr << StrFormat("Welcome to %s (%s) by @%s.", __pc_PCName, __pc_PCVersion, __pc_PCAuthor) << endl;
    cerr << "---" << endl;
    cerr << "[>] How to use?" << endl;
    cerr << "- const <name of new constant to be added> <value> <description>" << endl;
    cerr << "- formula <formula to be added> <description>" << endl;
    cerr << "- func <name of function to be added>" << endl;
    cerr << "- solve <argument 1> <argument 2> ... <root (eg. if you want to solve for T, type ?T)>" << endl;
    cerr << "- var <name of variable to be added> <description>" << endl;
    cerr << endl;
    cerr << "= consts   [ list all constants ]" << endl;
    cerr << "= formulas [ list all formulas  ]" << endl;
    cerr << "= funcs    [ list all functions ]" << endl;
    cerr << "= vars     [ list all variables ]" << endl;
    cerr << endl;
    cerr << "+ reload  |  reload all files" << endl;
    cerr << "+ save    |  save all to files" << endl;
    cerr << endl;
    cerr << "# quit" << endl;
    cerr << endl;
    cerr << "[>] Note:" << endl;
    cerr << "Should it not been for the syntax, please don't use spaces randomly." << endl;
    cerr << "Every name of constants, functions... must start with a letter and contain only numbers and letters." << endl;
    cerr << "There will be less error reported so you must be cautious about your typing." << endl;
    cerr << "Be careful when you add new entry. You will have to edit the log file if you make a mistake." << endl;
    cerr << "YOU are responsible for the program's misbehaviors!" << endl;
    cerr << "---" << endl;
}

void Report (Stats& __pc_Ends)
{
    clrscr;
    __pc_Ends.Export();
    __pc_getch();
}

int main ()
{
    Stats __pc_Ends = Stats();
    PhysicsCalculator(__pc_Ends);
    Report(__pc_Ends);
    return 0;
}
