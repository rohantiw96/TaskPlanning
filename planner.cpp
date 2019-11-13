#include <iostream>
#include <fstream>
#include <boost/functional/hash.hpp>
#include <regex>
#include <unordered_set>
#include <set>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <queue>

#define SYMBOLS 0
#define INITIAL 1
#define GOAL 2
#define ACTIONS 3
#define ACTION_DEFINITION 4
#define ACTION_PRECONDITION 5
#define ACTION_EFFECT 6

class GroundedCondition;
class Condition;
class GroundedAction;
class Action;
class Env;

using namespace std;

bool print_status = true;

class GroundedCondition
{
private:
    string predicate;
    list<string> arg_values;
    bool truth = true;

public:
    GroundedCondition(string predicate, list<string> arg_values, bool truth = true)
    {
        this->predicate = predicate;
        this->truth = truth;  // fixed
        for (string l : arg_values)
        {
            this->arg_values.push_back(l);
        }
    }

    GroundedCondition(const GroundedCondition& gc)
    {
        this->predicate = gc.predicate;
        this->truth = gc.truth;  // fixed
        for (string l : gc.arg_values)
        {
            this->arg_values.push_back(l);
        }
    }

    string get_predicate() const
    {
        return this->predicate;
    }
    list<string> get_arg_values() const
    {
        return this->arg_values;
    }

    bool get_truth() const
    {
        return this->truth;
    }

    friend ostream& operator<<(ostream& os, const GroundedCondition& pred)
    {
        os << pred.toString() << " ";
        return os;
    }

    bool operator==(const GroundedCondition& rhs) const
    {
        if (this->predicate != rhs.predicate || this->arg_values.size() != rhs.arg_values.size())
            return false;

        auto lhs_it = this->arg_values.begin();
        auto rhs_it = rhs.arg_values.begin();

        while (lhs_it != this->arg_values.end() && rhs_it != rhs.arg_values.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }

        if (this->truth != rhs.get_truth()) // fixed
            return false;

        return true;
    }

    string toString() const
    {
        string temp = "";
        temp += this->predicate;
        temp += "(";
        for (string l : this->arg_values)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct GroundedConditionComparator
{
    bool operator()(const GroundedCondition& lhs, const GroundedCondition& rhs) const
    {
        return lhs == rhs;
    }
};

struct GroundedConditionHasher
{
    size_t operator()(const GroundedCondition& gcond) const
    {
        return hash<string>{}(gcond.toString());
    }
};

typedef unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> condition_set;

class Condition
{
private:
    string predicate;
    list<string> args;
    bool truth;

public:
    Condition(string pred, list<string> args, bool truth)
    {
        this->predicate = pred;
        this->truth = truth;
        for (string ar : args)
        {
            this->args.push_back(ar);
        }
    }

    string get_predicate() const
    {
        return this->predicate;
    }

    list<string> get_args() const
    {
        return this->args;
    }

    bool get_truth() const
    {
        return this->truth;
    }

    friend ostream& operator<<(ostream& os, const Condition& cond)
    {
        os << cond.toString() << " ";
        return os;
    }

    bool operator==(const Condition& rhs) const // fixed
    {

        if (this->predicate != rhs.predicate || this->args.size() != rhs.args.size())
            return false;

        auto lhs_it = this->args.begin();
        auto rhs_it = rhs.args.begin();

        while (lhs_it != this->args.end() && rhs_it != rhs.args.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }

        if (this->truth != rhs.get_truth())
            return false;

        return true;
    }

    string toString() const
    {
        string temp = "";
        if (!this->truth)
            temp += "!";
        temp += this->predicate;
        temp += "(";
        for (string l : this->args)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct ConditionComparator
{
    bool operator()(const Condition& lhs, const Condition& rhs) const
    {
        return lhs == rhs;
    }
};

struct ConditionHasher
{
    size_t operator()(const Condition& cond) const
    {
        return hash<string>{}(cond.toString());
    }
};

class Action
{
private:
    string name;
    list<string> args;
    unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions;
    unordered_set<Condition, ConditionHasher, ConditionComparator> effects;

public:
    Action(string name, list<string> args,
        unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions,
        unordered_set<Condition, ConditionHasher, ConditionComparator> effects)
    {
        this->name = name;
        for (string l : args)
        {
            this->args.push_back(l);
        }
        for (Condition pc : preconditions)
        {
            this->preconditions.insert(pc);
        }
        for (Condition pc : effects)
        {
            this->effects.insert(pc);
        }
    }
    string get_name() const
    {
        return this->name;
    }
    list<string> get_args() const
    {
        return this->args;
    }
    unordered_set<Condition, ConditionHasher, ConditionComparator> get_preconditions() const
    {
        return this->preconditions;
    }
    unordered_set<Condition, ConditionHasher, ConditionComparator> get_effects() const
    {
        return this->effects;
    }

    bool operator==(const Action& rhs) const
    {
        if (this->get_name() != rhs.get_name() || this->get_args().size() != rhs.get_args().size())
            return false;

        return true;
    }

    friend ostream& operator<<(ostream& os, const Action& ac)
    {
        os << ac.toString() << endl;
        os << "Precondition: ";
        for (Condition precond : ac.get_preconditions())
            os << precond;
        os << endl;
        os << "Effect: ";
        for (Condition effect : ac.get_effects())
            os << effect;
        os << endl;
        return os;
    }

    string toString() const
    {
        string temp = "";
        temp += this->get_name();
        temp += "(";
        for (string l : this->get_args())
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct ActionComparator
{
    bool operator()(const Action& lhs, const Action& rhs) const
    {
        return lhs == rhs;
    }
};

struct ActionHasher
{
    size_t operator()(const Action& ac) const
    {
        return hash<string>{}(ac.get_name());
    }
};

class Env
{
private:
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> initial_conditions;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> goal_conditions;
    unordered_set<Action, ActionHasher, ActionComparator> actions;
    unordered_set<string> symbols;

public:
    void remove_initial_condition(GroundedCondition gc)
    {
        this->initial_conditions.erase(gc);
    }
    void add_initial_condition(GroundedCondition gc)
    {
        this->initial_conditions.insert(gc);
    }
    void add_goal_condition(GroundedCondition gc)
    {
        this->goal_conditions.insert(gc);
    }
    void remove_goal_condition(GroundedCondition gc)
    {
        this->goal_conditions.erase(gc);
    }
    void add_symbol(string symbol)
    {
        symbols.insert(symbol);
    }
    void add_symbols(list<string> symbols)
    {
        for (string l : symbols)
            this->symbols.insert(l);
    }
    void add_action(Action action)
    {
        this->actions.insert(action);
    }

    Action get_action(string name)
    {
        for (Action a : this->actions)
        {
            if (a.get_name() == name)
                return a;
        }
        throw runtime_error("Action " + name + " not found!");
    }
    unordered_set<string> get_symbols() const
    {
        return this->symbols;
    }

    condition_set get_inital_conditions() const
    {
        return this->initial_conditions;
    }

    condition_set get_goal_conditions() const
    {
        return this->goal_conditions;
    }

    unordered_set<Action, ActionHasher, ActionComparator> get_all_actions() const{
        return this->actions;
    }

    friend ostream& operator<<(ostream& os, const Env& w)
    {
        os << "***** Environment *****" << endl << endl;
        os << "Symbols: ";
        for (string s : w.get_symbols())
            os << s + ",";
        os << endl;
        os << "Initial conditions: ";
        for (GroundedCondition s : w.initial_conditions)
            os << s;
        os << endl;
        os << "Goal conditions: ";
        for (GroundedCondition g : w.goal_conditions)
            os << g;
        os << endl;
        os << "Actions:" << endl;
        for (Action g : w.actions)
            os << g << endl;
        cout << "***** Environment Created! *****" << endl;
        return os;
    }
};

class GroundedAction
{
private:
    string name;
    list<string> arg_values;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> gPreconditions;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> gEffects;

public:
    GroundedAction(string name, list<string> arg_values,unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> preconditions,unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> effects)
    {
        this->name = name;
        for (string ar : arg_values)
        {
            this->arg_values.push_back(ar);
        }
        for (GroundedCondition pc : preconditions)
        {
            this->gPreconditions.insert(pc);
        }
        for (GroundedCondition pc : effects)
        {
            this->gEffects.insert(pc);
        }
    }

    string get_name() const
    {
        return this->name;
    }

    list<string> get_arg_values() const
    {
        return this->arg_values;
    }

    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>  get_preconditions() const
    {
        return this->gPreconditions;
    }
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>  get_effects() const
    {
        return this->gEffects;
    }

    bool operator==(const GroundedAction& rhs) const
    {
        if (this->name != rhs.name || this->arg_values.size() != rhs.arg_values.size())
            return false;

        auto lhs_it = this->arg_values.begin();
        auto rhs_it = rhs.arg_values.begin();

        while (lhs_it != this->arg_values.end() && rhs_it != rhs.arg_values.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }
        return true;
    }

    friend ostream& operator<<(ostream& os, const GroundedAction& gac)
    {
        os << gac.toString() << " ";
        return os;
    }

    string toString() const
    {
        string temp = "";
        temp += this->name;
        temp += "(";
        for (string l : this->arg_values)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

list<string> parse_symbols(string symbols_str)
{
    list<string> symbols;
    size_t pos = 0;
    string delimiter = ",";
    while ((pos = symbols_str.find(delimiter)) != string::npos)
    {
        string symbol = symbols_str.substr(0, pos);
        symbols_str.erase(0, pos + delimiter.length());
        symbols.push_back(symbol);
    }
    symbols.push_back(symbols_str);
    return symbols;
}

Env* create_env(char* filename)
{
    ifstream input_file(filename);
    Env* env = new Env();
    regex symbolStateRegex("symbols:", regex::icase);
    regex symbolRegex("([a-zA-Z0-9_, ]+) *");
    regex initialConditionRegex("initialconditions:(.*)", regex::icase);
    regex conditionRegex("(!?[A-Z][a-zA-Z_]*) *\\( *([a-zA-Z0-9_, ]+) *\\)");
    regex goalConditionRegex("goalconditions:(.*)", regex::icase);
    regex actionRegex("actions:", regex::icase);
    regex precondRegex("preconditions:(.*)", regex::icase);
    regex effectRegex("effects:(.*)", regex::icase);
    int parser = SYMBOLS;

    unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions;
    unordered_set<Condition, ConditionHasher, ConditionComparator> effects;
    string action_name;
    string action_args;

    string line;
    if (input_file.is_open())
    {
        while (getline(input_file, line))
        {
            string::iterator end_pos = remove(line.begin(), line.end(), ' ');
            line.erase(end_pos, line.end());

            if (line == "")
                continue;

            if (parser == SYMBOLS)
            {
                smatch results;
                if (regex_search(line, results, symbolStateRegex))
                {
                    line = line.substr(8);
                    sregex_token_iterator iter(line.begin(), line.end(), symbolRegex, 0);
                    sregex_token_iterator end;

                    env->add_symbols(parse_symbols(iter->str()));  // fixed

                    parser = INITIAL;
                }
                else
                {
                    cout << "Symbols are not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == INITIAL)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, initialConditionRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        if (predicate[0] == '!')
                        {
                            env->remove_initial_condition(
                                GroundedCondition(predicate.substr(1), parse_symbols(args)));
                        }
                        else
                        {
                            env->add_initial_condition(
                                GroundedCondition(predicate, parse_symbols(args)));
                        }
                    }

                    parser = GOAL;
                }
                else
                {
                    cout << "Initial conditions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == GOAL)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, goalConditionRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        if (predicate[0] == '!')
                        {
                            env->remove_goal_condition(
                                GroundedCondition(predicate.substr(1), parse_symbols(args)));
                        }
                        else
                        {
                            env->add_goal_condition(
                                GroundedCondition(predicate, parse_symbols(args)));
                        }
                    }

                    parser = ACTIONS;
                }
                else
                {
                    cout << "Goal conditions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTIONS)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, actionRegex))
                {
                    parser = ACTION_DEFINITION;
                }
                else
                {
                    cout << "Actions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_DEFINITION)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, conditionRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;
                    // name
                    action_name = iter->str();
                    iter++;
                    // args
                    action_args = iter->str();
                    iter++;

                    parser = ACTION_PRECONDITION;
                }
                else
                {
                    cout << "Action not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_PRECONDITION)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, precondRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        bool truth;

                        if (predicate[0] == '!')
                        {
                            predicate = predicate.substr(1);
                            truth = false;
                        }
                        else
                        {
                            truth = true;
                        }

                        Condition precond(predicate, parse_symbols(args), truth);
                        preconditions.insert(precond);
                    }

                    parser = ACTION_EFFECT;
                }
                else
                {
                    cout << "Precondition not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_EFFECT)
            {
                const char* line_c = line.c_str();
                if (regex_match(line_c, effectRegex))
                {
                    const std::vector<int> submatches = { 1, 2 };
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        bool truth;

                        if (predicate[0] == '!')
                        {
                            predicate = predicate.substr(1);
                            truth = false;
                        }
                        else
                        {
                            truth = true;
                        }

                        Condition effect(predicate, parse_symbols(args), truth);
                        effects.insert(effect);
                    }

                    env->add_action(
                        Action(action_name, parse_symbols(action_args), preconditions, effects));

                    preconditions.clear();
                    effects.clear();
                    parser = ACTION_DEFINITION;
                }
                else
                {
                    cout << "Effects not specified correctly." << endl;
                    throw;
                }
            }
        }
        input_file.close();
    }

    else
        cout << "Unable to open file";

    return env;
}

unordered_set<string> get_arguements(condition_set& conditions){
    unordered_set<string> args;
    for(const GroundedCondition& c:conditions){
        for(const string str:c.get_arg_values()){
            args.insert(str);
        }
    }
    return args;
}

vector<pair<string,int>> get_actions(unordered_set<Action, ActionHasher, ActionComparator> action_set){
    vector<pair<string,int>> actions;
    for(const Action& act:action_set){
        actions.push_back(make_pair(act.get_name(),act.get_args().size()));
    }
    return actions;
}

// vector<list<string>> getPermuations(vector<string> arguments,int num_arguments){
//     vector<list<string>> argument_permutations;
//     if (num_arguments == 2){
//         for(int i=0;i<arguments.size();i++){
//             for (int j=0;j<arguments.size();j++){
//                 if (i != j){
//                     list<string> arg;
//                     arg.push_back(arguments[i]);
//                     arg.push_back(arguments[j]);
//                     argument_permutations.push_back(arg);
//                 }
//             }
//         }
//     }
//     else {
//         for(int i=0;i<arguments.size();i++){
//             for (int j=0;j<arguments.size();j++){
//                 for (int k=0;k<arguments.size();k++){
//                 if (i != j && j!=k && i!=k){
//                     list<string> arg;
//                     arg.push_back(arguments[i]);
//                     arg.push_back(arguments[j]);
//                     arg.push_back(arguments[k]);
//                     argument_permutations.push_back(arg);
//                     }
//                 }
//             }
//         }
//     }
//     return argument_permutations;
// }
void swap(vector<string>& arguments,int i,int j){
        string temp = arguments[i];
        arguments[i] = arguments[j];
        arguments[j] = temp;
}

void enumerate(vector<string>& arguments,int n,int k,vector<list<string>>& permutations){
        if (k == 0) {
            list<string> singlePermutation;
            for (int i = n; i < arguments.size(); i++){
                singlePermutation.push_back(arguments[i]);
            }
            permutations.push_back(singlePermutation);
            return;
        }

      for (int i = 0; i < n; i++) {
            swap(arguments, i, n-1);
            enumerate(arguments, n-1, k-1, permutations);
            swap(arguments, i, n-1);
        }

}

vector<list<string>> getPermuations(vector<string> arguments,int num_arguments){
    vector<list<string>> permutations;
    enumerate(arguments,arguments.size(),num_arguments,permutations);
    for(const auto& v:permutations){
        for(const auto& l:v){
            cout << l << " ";
        }
        cout << "\n";
    }
    return permutations;
}



bool checkPreconditions(condition_set state,condition_set precondition){
    for(const auto p:precondition){
        if(state.find(p) == state.end()) return false;
    }
    return true;
}

condition_set* applyAction(condition_set state,condition_set effect){
    condition_set* new_state = new condition_set;
    *new_state = state;
    for(const auto e:effect){
        GroundedCondition cond = GroundedCondition(e.get_predicate(),e.get_arg_values());
        if(!e.get_truth())
            new_state->erase(cond);
        else
            new_state->insert(cond);
    }
    return new_state;
}

GroundedAction* getGroundedAction(Action act,list<string> arg){
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> preconditions;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> effects;
    list<string> action_arguments = act.get_args();
    for(const Condition cond:act.get_preconditions()){
        list<string> condition_arguments = cond.get_args();
        list<string> grounded_condition_arguments = condition_arguments;
        for(int i=0;i<condition_arguments.size();i++){
            for(int j=0;j<action_arguments.size();j++){
                if (*std::next(condition_arguments.begin(), i) == *std::next(action_arguments.begin(), j)){
                    *std::next(grounded_condition_arguments.begin(), i) = *std::next(arg.begin(), j);
                }
            }
        }
        preconditions.insert(GroundedCondition(cond.get_predicate(),grounded_condition_arguments,cond.get_truth()));
    }
    for(const Condition cond:act.get_effects()){
        list<string> condition_arguments = cond.get_args();
        list<string> grounded_condition_arguments = condition_arguments;
        for(int i=0;i<condition_arguments.size();i++){
            for(int j=0;j<action_arguments.size();j++){
                if (*std::next(condition_arguments.begin(), i) == *std::next(action_arguments.begin(), j)){
                    *std::next(grounded_condition_arguments.begin(), i) = *std::next(arg.begin(), j);
                }
            }
        }
        effects.insert(GroundedCondition(cond.get_predicate(),grounded_condition_arguments,cond.get_truth()));
    }
    return new GroundedAction(act.get_name(),arg,preconditions,effects);
}

string stateToString(condition_set node){
    vector<string> state;
    string stateString;
    for(const auto& c:node){
        state.push_back(c.toString());
    }
    sort(state.begin(),state.end());
    for(const auto& c:state){
        stateString+=c;
    }
    return stateString;
}

bool checkGoal(condition_set current, condition_set goal){
    for(const auto& c:goal){
        if (current.find(c) == current.end()) return false;
    }
    return true;
}
struct Node{
    condition_set state_;
    double g_value_;
    double f_value_;
    GroundedAction* action_;
    Node(condition_set state,GroundedAction* action,double g_value,double f_value): state_(state),action_(action), g_value_(g_value),f_value_(f_value) {}

};
struct CompareNode { 
    bool operator()(Node* const& n1, Node* const& n2) 
    { 
        // return "true" if "p1" is ordered  
        // before "p2", for example: 
        return n1->f_value_ > n2->f_value_; 
    } 
}; 
  

list<GroundedAction*>* backTrack(unordered_map<string,Node*>& came_from,condition_set& start,Node* goal){
    list<GroundedAction*>* actions = new list<GroundedAction*>;
    Node* current_state = goal;
    while (stateToString(current_state->state_) != stateToString(start)) // Backtracking to get the shortest path
    {
        // for (const auto& c:current_state->state_){
        //     cout << c.toString() << " ";
        // }
        // cout << "\n";
        cout << stateToString(current_state->state_) << endl;
        cout << current_state->action_->toString() << endl;
        actions->push_back(current_state->action_);
        current_state = came_from[stateToString(current_state->state_)];
        
    }
    cout << stateToString(start) << endl;
    return actions;
}


bool expandActionsAndArguments(queue<Node*>& que,unordered_map<string,bool>& visited,unordered_set<Action, ActionHasher, ActionComparator>& action_set,unordered_map<int,vector<list<string>>>& argument_permutations,unordered_map<string,Node*>& came_from,Node*& current_node,Node*& goal_node){
    for(const Action act:action_set){
        for(const list<string> arg:argument_permutations[act.get_args().size()]){
            GroundedAction* grounded_action = getGroundedAction(act,arg);
            if (checkPreconditions(current_node->state_,grounded_action->get_preconditions())){
                condition_set* neighbour = applyAction(current_node->state_,grounded_action->get_effects());
                if (!visited[stateToString(*neighbour)]){
                    visited[stateToString(*neighbour)] = true;
                    Node* neighbour_node = new Node(*neighbour,grounded_action,current_node->g_value_+1,0);
                    came_from[stateToString(*neighbour)] = current_node;
                    que.push(neighbour_node);
                    if (checkGoal(*neighbour,goal_node->state_)){
                        goal_node = neighbour_node;
                        cout << "Path Found" << endl;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

double getHeuristic(condition_set node,condition_set goal){
    double count  = 0;
    for(const auto goal_conditions:goal){
        for(const auto conditions:node){
            if (goal_conditions == conditions){
                count++;
            }
        }
    }
    return goal.size() - count;
    // return 0;
}


void expandActionsAndArgumentsAstar(priority_queue<Node*,vector<Node*>,CompareNode>& open_list,unordered_set<string>& closed_list,unordered_set<Action, ActionHasher, ActionComparator>& action_set,unordered_map<int,vector<list<string>>>& argument_permutations,unordered_map<string,Node*>& came_from,Node*& current_node,Node*& goal_node,double edge_cost){
    for(const Action act:action_set){
        for(const list<string> arg:argument_permutations[act.get_args().size()]){
            GroundedAction* grounded_action = getGroundedAction(act,arg);
            if (checkPreconditions(current_node->state_,grounded_action->get_preconditions())){
                condition_set* neighbour = applyAction(current_node->state_,grounded_action->get_effects());
                if (closed_list.find(stateToString(*neighbour)) == closed_list.end()){
                    Node* neighbour_node = new Node(*neighbour,grounded_action,INT_MAX,INT_MAX);
                    if(neighbour_node->g_value_ > current_node->g_value_ + edge_cost){
                        neighbour_node->g_value_ =  current_node->g_value_ + edge_cost;
                        neighbour_node->f_value_ = neighbour_node->g_value_ + 5 * getHeuristic(*neighbour,goal_node->state_);
                        open_list.push(neighbour_node);
                        came_from[stateToString(*neighbour)] = current_node;
     
                    }
                }
            }
        }
    }
}

list<GroundedAction> planner(Env* env)
{
    // this is where you insert your planner
    condition_set start = env->get_inital_conditions();
    condition_set goal = env->get_goal_conditions();
    unordered_set<Action, ActionHasher, ActionComparator> action_set = env->get_all_actions();
    
    unordered_set<string> arguments = get_arguements(start);
    vector<string> args;
    args.insert(args.end(),arguments.begin(),arguments.end());

    vector<pair<string,int>> action_names = get_actions(env->get_all_actions());
    unordered_map<int,vector<list<string>>> argument_permutations;
    for(int i=0;i<action_names.size();i++){
        if(argument_permutations.find(action_names[i].second) ==  argument_permutations.end()){
            argument_permutations[action_names[i].second] = getPermuations(args,action_names[i].second);
        }
    }

    queue<Node*> que;
    unordered_map<string,Node*> came_from;
    unordered_map<string,bool> visited;
    Node* start_node = new Node(start,NULL,0,0);
    Node* goal_node = new Node(goal,NULL,INT_MAX,0); 
    Node* current_node = start_node;

    que.push(start_node);
    visited[stateToString(start_node->state_)] = true;
    while(!que.empty()){
        current_node = que.front();
        que.pop();
        if (expandActionsAndArguments(que,visited,action_set,argument_permutations,came_from,current_node,goal_node)) break;
    }
    list<GroundedAction*>* path = backTrack(came_from,start,goal_node);
    list<GroundedAction> actions;
    std::transform(std::begin(*path), std::end(*path),std::front_inserter(actions),[](GroundedAction* item){return *item;});
    return actions;
}


list<GroundedAction> plannerAstar(Env* env)
{
    // this is where you insert your planner
    condition_set start = env->get_inital_conditions();
    condition_set goal = env->get_goal_conditions();
    unordered_set<Action, ActionHasher, ActionComparator> action_set = env->get_all_actions();
    
    unordered_set<string> arguments = get_arguements(start);
    vector<string> args;
    args.insert(args.end(),arguments.begin(),arguments.end());

    vector<pair<string,int>> action_names = get_actions(env->get_all_actions());
    unordered_map<int,vector<list<string>>> argument_permutations;
    for(int i=0;i<action_names.size();i++){
        if(argument_permutations.find(action_names[i].second) ==  argument_permutations.end()){
            argument_permutations[action_names[i].second] = getPermuations(args,action_names[i].second);
        }
    }


    unordered_map<string,Node*> came_from;
    Node* start_node = new Node(start,NULL,0,getHeuristic(start,goal));
    Node* goal_node = new Node(goal,NULL,INT_MAX,0);  
    Node* current_node = start_node;
    priority_queue<Node*,vector<Node*>,CompareNode> open_list;
    unordered_set<string> closed_list;
    double edge_cost = 1;
    open_list.push(start_node);

    while(!open_list.empty()){
        current_node = open_list.top();
        open_list.pop();
        closed_list.insert(stateToString(current_node->state_));
        if (checkGoal(current_node->state_,goal)){
            cout << "Path Found" << endl;
            goal_node = current_node;
            break;
        }
        expandActionsAndArgumentsAstar(open_list,closed_list,action_set,argument_permutations,came_from,current_node,goal_node,edge_cost);
    }
    cout << "Backtracking" << endl;
    list<GroundedAction*>*  path = backTrack(came_from,start,goal_node);
    list<GroundedAction> actions;
    std::transform(std::begin(*path), std::end(*path),std::front_inserter(actions),[](GroundedAction* item){return *item;});
    return actions;
}
int main(int argc, char* argv[])
{
    // DO NOT CHANGE THIS FUNCTION
    char* filename = (char*)("fire.txt");
    if (argc > 1)
        filename = argv[1];

    cout << "Environment: " << filename << endl << endl;
    Env* env = create_env(filename);
    // if (print_status)
    // {
    //     cout << *env;
    // }

    list<GroundedAction> actions = plannerAstar(env);

    cout << "\nPlan: " << endl;
    for (GroundedAction gac : actions)
    {
        cout << gac << endl;
    }

    return 0;
}