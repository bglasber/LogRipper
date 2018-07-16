from read_transition_dump import *
import pytest

def test_build_simple_arr():
    transitions = {}
    transitions[1] = transition_node( 1, "test1" )
    transitions[1].add_transition( 1, 0.5 )
    transitions[1].add_transition( 2, 0.5 )
    transitions[2] = transition_node( 2, "test2" )
    transitions[2].add_transition( 1, 0.5 )
    transitions[2].add_transition( 2, 0.5 )
    
    ordered_keys = [1,2]
    flattened_arr = convert_transitions_to_np_arr( ordered_keys, transitions )
    assert (flattened_arr == 0.5).all()

def test_build_3_3_arr():
    transitions = {}
    transitions[1] = transition_node( 1, "test1" )
    transitions[1].add_transition( 1, 0.5 )
    transitions[1].add_transition( 2, 0.1 )
    transitions[1].add_transition( 3, 0.4 )
    transitions[2] = transition_node( 2, "test2" )
    transitions[2].add_transition( 1, 0.3 )
    transitions[2].add_transition( 2, 0.5 )
    transitions[2].add_transition( 3, 0.2 )
    transitions[3] = transition_node( 3, "test3" )
    transitions[3].add_transition( 1, 0.1 )
    transitions[3].add_transition( 2, 0.8 )
    transitions[3].add_transition( 3, 0.1 )
 
    
    ordered_keys = [1,2,3]
    flattened_arr = convert_transitions_to_np_arr( ordered_keys, transitions )
    assert flattened_arr[0] == flattened_arr[4] == 0.5
    assert flattened_arr[1] == flattened_arr[6] == flattened_arr[8] == 0.1
    assert flattened_arr[2] == 0.4
    assert flattened_arr[3] == 0.3
    assert flattened_arr[5] == 0.2
    assert flattened_arr[7] == 0.8

def test_missing_entries():
    transitions = {}
    transitions[1] = transition_node( 1, "test1" )
    transitions[1].add_transition( 1, 0.5 )
    transitions[1].add_transition( 3, 0.5 )
    transitions[2] = transition_node( 2, "test2" )
    transitions[2].add_transition( 2, 0.5 )
    transitions[2].add_transition( 3, 0.5 )
    transitions[3] = transition_node( 3, "test3" )
    transitions[3].add_transition( 1, 1. )

    ordered_keys = [1,2,3]
    flattened_arr = convert_transitions_to_np_arr( ordered_keys, transitions )
    assert flattened_arr[0] == flattened_arr[2] == 0.5
    assert flattened_arr[4] == flattened_arr[5] == 0.5
    assert flattened_arr[6] == 1.
    assert flattened_arr[1] == flattened_arr[3] == flattened_arr[7] == flattened_arr[8] == 0.0

def test_cpd_calc_simple():
    arr1 = np.array( [0.5, 0.5, 0.5, 0.5] )
    arr2 = np.array( [0.5, 0.5, 0.5, 0.5] )
    assert cpd_divergence_calc( arr1, arr2 ) == 0.0

def test_cpd_calc_divergence():
    arr1 = np.array( [0.75, 0.25, 0.25, 0.75] )
    arr2 = np.array( [0.5, 0.5, 0.5, 0.5] )
    assert cpd_divergence_calc( arr1, arr2 ) == 4 * 0.25**2

def test_cpd_calc_symmetric():
    arr1 = np.array( [0.75, 0.25, 0.25, 0.75] )
    arr2 = np.array( [0.25, 0.75, 0.75, 0.25] )
    assert cpd_divergence_calc( arr1, arr2 ) == 4 * 0.5**2




test_cpd_calc_simple()


