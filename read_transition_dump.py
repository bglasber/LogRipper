import numpy as np

class transition_node:

    def __init__( self, key, prob, val ):
        self.key = key
        self.prob = prob
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
                prob = float(sp[1].lstrip())
                line_val = ":".join( sp[2:] )
                key_to_str_map[k] = transition_node( k, prob, line_val.strip() )
    return key_to_str_map

def find_key_ind( key, ordered_keys ):
    for i in range(len(ordered_keys)):
        if ordered_keys[i] == key:
            return i
    return None

def convert_transition_graph_into_sendable_form( tgraph ):
    # There are three things we want to send
    unique_log_ids = []
    log_id_probs = []
    log_id_transition_probs = []

    unique_log_ids = list(tgraph.keys())

    for log_id in unique_log_ids:
        log_id_probs.append( tgraph[log_id].prob )

    for log_id in unique_log_ids:
        for transition, prob in tgraph[log_id].node_transitions.items():
            log_id_transition_probs.append( (log_id, transition, prob ) )

    return [ unique_log_ids, log_id_probs, log_id_transition_probs ]

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

print( convert_transition_graph_into_sendable_form( extract_transitions_from_dump( "out" ) ) )
