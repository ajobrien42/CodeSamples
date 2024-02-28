# ktape.py
# Simulates a k-tape Turing Machine. 
# Given a TM and an input string, determines whether the machine will accept or reject the string within a given step limit.

import sys


class State:

    def __init__(self, label_in='-'):
    
        self.label = label_in
        self.transitions = list() # a list of lists of lists and strings: [[chars of transition for each k], 'state destination char', [rewrite chars for each k], [direction for each k]]
        self.start_state = False
        self.reject_state = False
        self.accept_state = False

    def __eq__(self, s):
        
        if (self.label == s.label) and (self.transitions == s.transitions) and (self.start_state == s.start_state) and (self.reject_state == s.reject_state) and (self.accept_state == s.accept_state):
            return True
        else:
            return False



# Like a state class, but keeps track of how we transitioned to the state
class Path_State:

    def __init__(self, state=None, transitioned_from=None, transitioners=None, rewrites=None, directions=None):

        self.state = state # a state STRING
        self.transitioned_from = transitioned_from # a list of state STRINGS
        self.transitioners = transitioners # a list of chars
        self.rewrites = rewrites # a list of chars (the character we rewrote to when getting here for each tape)
        self.directions = directions # a list of chars (the direction we took to get here for all tapes)
        self.visited = list() # a list of lists: ['char of transition', 'state destination char', 'rewrite char', 'direction']

    def __eq__(self, p):
        
        if (self.state == p.state) and (self.transitioned_from == p.transitioned_from) and (self.transitioners == p.transitioners) and (self.visited == p.visited):
            return True
        else:
            return False



class TM:

    def __init__(self, name_in='-'):

        self.name = name_in
        self.k = 1  # number of tapes
        self.states = dict() # key: string, value: state object
        self.start_state = None # string
        self.reject_state = None # string
        self.accept_state = None #string

    def readFile(self, filename):

        # open file and read first lines
        rfile = open(filename, "r")

        # set the name of the machine and number of tapes
        first_line = rfile.readline().strip().split(',')
        self.name = first_line[0]
        self.k = first_line[1]

        # create the states
        the_states = rfile.readline().strip().split(',')
        for s in the_states:
            temp = State(s)
            self.states[s] = temp

        # skip the alphabet lines
        rfile.readline()
        rfile.readline()

        # get start and accept states
        start_state_string = rfile.readline().strip()
        accept_state_string = rfile.readline().strip()
        self.states[start_state_string].start_state = True
        self.states[accept_state_string].accept_state = True
        self.start_state = start_state_string
        self.accept_state = accept_state_string

        # check reject state
        reject_state_string = rfile.readline().strip()
        if reject_state_string != 'qreject':
            self.states[reject_state_string].reject_state = True
        self.reject_state = reject_state_string

        # read in transitions
        curr = rfile.readline()
        k = int(self.k)

        while curr:

            curr = curr.strip().split(',')

            trans_chars = []
            rewrites = []
            directions = []

            # read in the transitions
            start_state_string = curr[0]

            for i in range(1, k+1):
                trans_chars.append(curr[i])

            end_state_string = curr[k+1]

            for i in range(k+2, 2*k+2):
                rewrites.append(curr[i])

            for i in range(2*k+2, 3*k+2):
                directions.append(curr[i])

            # add transition to state object
            trans_append = [trans_chars, end_state_string, rewrites, directions]
            self.states[start_state_string].transitions.append(trans_append)

            # read next line
            curr = rfile.readline()

        rfile.close()

        return



    def print(self):

        print(self.name)
        print("Number of tapes: " + self.k)
        print("Start state: " + self.start_state)
        print("Accept state: " + self.accept_state)
        if self.reject_state == None:
            print("Reject state: qreject")
        else:
            print("Reject state: " + self.reject_state)

        print("Transitions: ")

        for s in self.states:
            for trans_list in self.states[s].transitions:
                
                transition_chars = ""
                for i in range(int(self.k)):
                    transition_chars += trans_list[0][i]

                rewrites = ""
                for i in range(int(self.k)):
                    rewrites += trans_list[2][i]

                directions = ""
                for i in range(int(self.k)):
                    directions += trans_list[3][i]
                
                print(s + "->" + trans_list[1] + ": " + transition_chars + " -> " + rewrites + ", " + directions)

        return



def find_path(tm, input_string, step_limit):

    # initialize variables
    k = int(tm.k)
    input_string = list(input_string)

    curr_tapes = []
    for i in range(k):
        curr_tapes.append(['-'] * (len(input_string)+1))

    for i in range(len(input_string)):
        curr_tapes[0][i] = input_string[i]

    curr_path = list()
    longest_path = list()
    total_transitions = 0
    ended_in_accept = False

    # create path state from start state
    start_path = Path_State(tm.start_state)
    curr_path.append(start_path)
    longest_path.append(start_path)

    # head positions for each tape
    head_pos = [0] * k

    # loop until path is empty
    while len(curr_path) > 0:

        # check if reached step limit
        if len(curr_path) > int(step_limit):
            return [], total_transitions, 'STEP LIMIT'

        # edit head_pos if needed
        for i in range(k):
            if head_pos[i] >= len(input_string):
                for i in range(head_pos[i]-len(input_string)+1):
                    curr_tapes[i].append('-')

        # if top is an accept state
        if tm.states[curr_path[-1].state].accept_state == True:

            return curr_path, total_transitions, 'ACCEPT'

        # if top is a reject state
        elif tm.states[curr_path[-1].state].reject_state == True:

            # move heads back one step
            for i in range(k):
                if curr_path[-1].directions[i] == 'L':
                    head_pos[i] += 1
                elif curr_path[-1].directions[i] == 'R':
                    head_pos[i] -= 1
                if head_pos[i] < 0:
                    head_pos[i] = 0

            # rewrite tape cells with previous values
            for i in range(k):
                curr_tapes[i][head_pos[i]] = curr_path[-1].transitioners[i]

            curr_path.pop()
    
        # need to decide whether to add a state or pop
        else:

            need_to_pop = True

            # check through transitions and try to find a valid one not in visited
            for trans_list in tm.states[curr_path[-1].state].transitions:
                # trans_list: [[chars of transition for each k], 'state destination char', [rewrite chars for each k], [direction for each k]]

                if (trans_list not in curr_path[-1].visited):

                    # check if all transition characters match the read characters
                    mismatch=False
                    for i in range(k):
                        if not trans_list[0][i] == curr_tapes[i][head_pos[i]]:
                            mismatch=True
                    if mismatch:
                        continue

                    # found valid transition: add and break
                    need_to_pop = False
                    curr_path[-1].visited.append(trans_list)

                    new_path_state = Path_State(trans_list[1], curr_path[-1].state, trans_list[0], trans_list[2], trans_list[3])
                    curr_path.append(new_path_state)

                    # fix tapes and heads
                    for i in range(k):
                        curr_tapes[i][head_pos[i]] = trans_list[2][i]
                        if trans_list[3][i] == 'R':
                            head_pos[i] += 1
                        elif trans_list[3][i] == 'L':
                            head_pos[i] -= 1

                    break

            if need_to_pop:

                # move heads back one step
                for i in range(k):
                    
                    if not curr_path[-1].directions:
                        return longest_path, total_transitions, 'REJECT'
                    if curr_path[-1].directions[i] == 'L':
                        head_pos[i] += 1
                    elif curr_path[-1].directions[i] == 'R':
                        head_pos[i] -= 1
                    if head_pos[i] < 0:
                        head_pos[i] = 0

                # rewrite tape
                for i in range(k):

                    curr_tapes[i][head_pos[i]] = curr_path[-1].transitioners[i]

                curr_path.pop()

        # edit longest path if needed
        if len(curr_path) >= len(longest_path):
            longest_path.clear()
            for elem in curr_path:
                longest_path.append(elem)

        total_transitions += 1

    if ended_in_accept:
        return curr_path, total_transitions, 'ACCEPT'
    else:
        return longest_path, total_transitions, 'REJECT'



if __name__=='__main__':

    if len(sys.argv) != 4:
        print("Incorrect number of inputs.")
        print("ktape.py [path to machine] [input string] [step limit]")
        exit()

    t = TM('what')
    t.readFile(sys.argv[1])
    print("\n----------------")
    t.print()
    print("----------------\n")

    print("Computing path...\n")
    input_string = sys.argv[2]
    step_limit = sys.argv[3]
    final_path, num_trans, result = find_path(t, input_string, step_limit)

    print("----------------")
    print("Input string: " + sys.argv[2])
    print("Total number of transitions taken: " + str(num_trans))

    # print depending on result
    if result == 'ACCEPT':

        print("String accepted in " + str(len(final_path)) + " states")
        for i in range(1, len(final_path)):
            print(final_path[i].transitioners, final_path[i].transitioned_from, ' -> ', final_path[i].state, final_path[i].rewrites)

    elif result == 'REJECT':
        print("String rejected in " + str(len(final_path)) + " states")
    else:
        print("Execution stopped after " + str(num_trans) + " steps")
