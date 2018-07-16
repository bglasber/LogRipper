import numpy as np

class transition_node:

    def __init__( self, key, val ):
        self.key = key
        self.val = val
        self.node_transitions = {}

    def add_transition( self, key, prob ):
        self.node_transitions[ key ] = prob

def extract_transitions_from_dump( fname ):
    key_to_str_map = {}
    with open( fname, "r" ) as f:
        for line in f:
            # Transition line
            if "->" in line:
                transition = line.split( "->" )
                key = int(transition[0])
                trans_prob= transition[1].split(":")
                t_key = int( trans_prob[0].lstrip() )
                prob = float( trans_prob[1].lstrip() )
                key_to_str_map[key].add_transition( t_key, prob )
            # Key value line
            elif ":" in line:

                # We only want to split on the first one, so piece it back together afterwards
                sp = line.split(":")
                k = int(sp[0])
                line_val = ":".join( sp[1:] )
                key_to_str_map[k] = transition_node( k, line_val.strip() )
    return key_to_str_map

def find_key_ind( key, ordered_keys ):
    for i in range(len(ordered_keys)):
        if ordered_keys[i] == key:
            return i
    return None

def convert_transitions_to_np_arr( ordered_keys, transitions ):
    mat = np.zeros( (len(ordered_keys), len(ordered_keys)) )
    for key in ordered_keys:
        row = find_key_ind( key, ordered_keys )
        row_transitions = transitions[ key ]
        for t_key, prob in row_transitions.node_transitions.items():
            col = find_key_ind( t_key, ordered_keys )
            mat[row][col] = prob
    return mat.ravel()

def cpd_divergence_calc( arr1, arr2 ):
    entries = arr1.shape[0]
    row_len = int( np.sqrt( entries ) )

    loc_arr1 = np.copy( arr1 )
    loc_arr2 = np.copy( arr2 )

    # Euclidean distance between cpds
    return np.sum( np.square( loc_arr1 - loc_arr2 ) )
