# nfa2dfa.py
# Converts a nondeterministic finite automata to a deterministic finite automata.

import sys
import csv
import itertools

# holds the information for an individual state
class State:

    def __init__(self, label):
        self.label = label          # string that holds state name
        self.start_state = False
        self.final_state = False
        self.transitions = dict()   # key: character of transition, value: list of State objects



# parent class for NFA and DFA
class Machine:

    # save stats of machine to csv
    def save_machine(self, filename):
        wfile = open(filename, "w", newline='')

        csvwriter = csv.writer(wfile)

        # print machine name
        csvwriter.writerow([self.name])

        # print the states
        state_labels = []

        for state_label in self.states:
            state_labels.append(state_label)

        csvwriter.writerow(state_labels)

        # print alphabet
        csvwriter.writerow(self.alphabet)

        # print start state
        csvwriter.writerow([self.start_state.label])

        # print final states
        state_labels = []

        for final_state in self.final_states:
            state_labels.append(final_state.label)

        csvwriter.writerow(state_labels)

        # print the transitions
        for state_string in self.states:
            for trans_char in self.states[state_string].transitions:
                for end_state in self.states[state_string].transitions[trans_char]:
                    csvwriter.writerow([self.states[state_string].label, trans_char, end_state.label])

        wfile.close()



# the information for an NFA machine
class NFA(Machine):

    def __init__(self):
        self.name = None            # Machine name
        self.states = dict()        # key: label, value: State object 
        self.alphabet = list()      # List of symbols in alphabet
        self.start_state = None     # State object
        self.final_states = list()  # List of State objects

    # reads in a CSV file and creates the machine
    def read_file(self, filename):

        rfile = open(filename, "r")

        # read in machine name
        self.name = rfile.readline().strip()

        # read in states
        temp_states = rfile.readline().strip().split(',')

        # initialize nodes for each state
        for s in temp_states:
            self.states[s] = State(s)

        # read in alphabet
        self.alphabet = rfile.readline().strip().split(',')

        # set start state
        start_state_string = rfile.readline().strip()
        self.states[start_state_string].start_state = True
        self.start_state = self.states[start_state_string]

        # set final states
        final_states_stringList = rfile.readline().strip().split(',')

        # loop through final states and set them
        for final_state_string in final_states_stringList:
            self.states[final_state_string].final_state = True
            self.final_states.append(self.states[final_state_string])

        # read through transitions
        curr = rfile.readline().strip()

        while curr:

            curr = curr.split(',')

            # initialize transition if needed
            if curr[1] not in self.states[curr[0]].transitions:
                self.states[curr[0]].transitions[curr[1]] = list()
        
            # append transition
            self.states[curr[0]].transitions[curr[1]].append(State(curr[2]))

            # read next line
            curr = rfile.readline().strip()



# the information for a DFA machine
class DFA(Machine):

    def __init__(self, nfa):
        self.name = nfa.name            # unchanged
        self.alphabet = nfa.alphabet    # unchanged

        # find states: every combination of original states and null
        self.states = dict()    # key: label, value: State object 

        combinations = []
        label_list = []

        for length in range(1, len(nfa.states)+1):
            for subset in itertools.combinations(nfa.states, length):
                combinations.append(subset)

        for combo in combinations:
            label_list.append('+'.join(combo))

        for label in label_list:
            self.states[label] = State(label)
        

        # find E(r) for all NFA states
        e_trans = dict()    # key: state label, val: list of reachable states by epsilon transitions

        # initialize each state with itself
        for state_label in nfa.states:
            e_trans[state_label] = [state_label]    

        # add epsilon transitions
        for state in nfa.states.items():
            for transition in state[1].transitions:
                if transition == '~':
                    for next_state in state[1].transitions['~']:
                        e_trans[state[0]].append(next_state.label)


        # find start state: E(NFA start state)
        start_state = '+'.join(e_trans[nfa.start_state.label])
        self.start_state = self.states[start_state]
        
        # set start state flag in self.states
        self.states[start_state].start_state = True


        # find final state: all states containing original final states
        self.final_states = list()      # List of state objects

        for state_label in self.states:
            for nfa_end in nfa.final_states:
                if nfa_end.label in state_label:
                    if not self.states[state_label] in self.final_states:
                        self.final_states.append(self.states[state_label])

                        # set final state flags in self.states
                        self.states[state_label].final_state = True


        # find transitions
        # assign next states to all dfa states as a list
        for nfa_state in nfa.states.items():
            for transition in nfa_state[1].transitions:
                for dfa_state in self.states.items():
                    if nfa_state[0] in dfa_state[0]:
                        t_list = nfa_state[1].transitions[transition]

                        if not transition in dfa_state[1].transitions:
                            dfa_state[1].transitions[transition] = []

                        dfa_state[1].transitions[transition].extend(t_list)

        # collect next states
        # e.g. if q1+q2 -- 0 -> q3 and q1+q2 -- 0 -> q1, then q1+q2 -- 0 -> q1+q3
        for state in self.states.items():
            for transition in state[1].transitions:
                next_states_list = state[1].transitions[transition]

                labels = []
                
                for s in next_states_list:
                    labels.append(s.label)

                # remove duplicates
                labels = [*set(labels)]

                labels.sort()

                state[1].transitions[transition] = [self.states['+'.join(labels)]]
        

        # use and remove epsilon transitions
        # create dictionary of epsilon transitions
        epsilon_transitions = dict()    # key: start state label, value: array of next States

        for state in self.states.items():
            if not state[0] in epsilon_transitions:
                epsilon_transitions[state[0]] = []

            if '~' in state[1].transitions:
                epsilon_transitions[state[0]].extend(state[1].transitions['~'])

        # apply epsilon transitions
        change = True  # used to repeat process until no new transitions are added

        while change:
            change = False
            for state in self.states.items():
                for transition_char in self.alphabet:

                    if not transition_char in state[1].transitions:
                        continue

                    if state[1].transitions[transition_char][0].label in epsilon_transitions:
                        next_states_list = []

                        for s in epsilon_transitions[state[0]]:
                            for sub_s in s.label.split('+'):
                                next_states_list.append(sub_s)
                        
                        for s in state[1].transitions[transition_char][0].label.split('+'):
                            next_states_list.append(s)

                        # if all parts of an epsilon transition start state are in a dfa transition's next state, 
                        # then the epsilon next states are also in the dfa transition's next state
                        for e_start_state in epsilon_transitions:
                            e_start_states_list = e_start_state.split('+')
                            for e_start_state in e_start_states_list:
                                if all(e_start_state in next_states_list for e_start_state in e_start_states_list):
                                    for e_transition in epsilon_transitions[e_start_state]:
                                        e_transition_list = e_transition.label.split('+')
                                        for e_next_state in e_transition_list:
                                            if not e_next_state in next_states_list:
                                                next_states_list.append(e_next_state)
                                                change=True 

                        # remove duplicates
                        next_states_list = [*set(next_states_list)]

                        next_states_list.sort()

                        label = '+'.join(next_states_list)
                        state[1].transitions[transition_char][0] = self.states[label]

        # remove epsilon transitions
        for state in self.states.values():
            state.transitions = {trans_char: next_states for trans_char, next_states in state.transitions.items() if trans_char != '~'}



def main():

    # check for valid input
    if len(sys.argv) == 2:
        in_file = sys.argv[1]
    else:
        print("Invalid number of inputs.")
        exit()

    # initialize NFA machine
    n = NFA()
    n.read_file(in_file)

    # initialize DFA machine
    d = DFA(n)

    # create write file
    out_file = "machines/" + (in_file.split('/')[1]).split('.')[0] + "_dfa.csv"

    # write DFA to csv file    
    d.save_machine(out_file)


if __name__=='__main__':
    main()
