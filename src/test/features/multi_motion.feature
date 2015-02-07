Feature: Shareholders can vote for multiple motions
  Scenario: Two voters and two motions
    Given a network with nodes "Alice" and "Bob" able to mint
    And the nodes travel to the Nu protocol v05 switch time
    When node "Alice" votes for the motions:
      | 3f786850e387550fdab836ed7e6dc881de23001b |
    When node "Bob" votes for the motions:
      | 89e6c98d92887913cadf06b2adb97f26cde4849b |
      | 3f786850e387550fdab836ed7e6dc881de23001b |
    And node "Alice" finds 5 blocks received by all nodes
    And node "Bob" finds 3 blocks received by all nodes
    Then motion "89e6c98d92887913cadf06b2adb97f26cde4849b" should have 3 blocks
    And motion "3f786850e387550fdab836ed7e6dc881de23001b" should have 8 blocks
    And motion "0000000000000000000000000000000000000000" should not have any vote

  Scenario: Multiple motions before protcol v05 switch time
    Given a network with nodes "Alice" able to mint
    And a node "v04" running version "0.4.2"
    When node "Alice" votes for the motions:
      | 89e6c98d92887913cadf06b2adb97f26cde4849b |
      | 3f786850e387550fdab836ed7e6dc881de23001b |
    And node "Alice" finds a block "X"
    Then the vote of block "X" on node "Alice" should return
      """
      {
        "custodians": [],
        "parkrates": [],
        "motions": ["89e6c98d92887913cadf06b2adb97f26cde4849b"],
        "fees": {}
      }
      """
    And all nodes should reach block "X"
    And the vote of block "X" on node "v04" should return
      """
      {
        "custodians": [],
        "parkrates": [],
        "motionhash": "89e6c98d92887913cadf06b2adb97f26cde4849b"
      }
      """
    And motion "89e6c98d92887913cadf06b2adb97f26cde4849b" should have 1 block
    And motion "3f786850e387550fdab836ed7e6dc881de23001b" should not have any vote

  Scenario: A v04 node finds a block on a network with a v05 node
    Given a network with nodes "Alice" able to mint
    And a node "v04" running version "0.4.2" and able to mint
    When node "v04" votes for the motion hash "2b66fd261ee5c6cfc8de7fa466bab600bcfe4f69"
    And node "v04" finds a block "X"
    Then the vote of block "X" on node "v04" should return
      """
      {
        "custodians": [],
        "parkrates": [],
        "motionhash": "2b66fd261ee5c6cfc8de7fa466bab600bcfe4f69"
      }
      """
    And all nodes should reach block "X"
    Then the vote of block "X" on node "Alice" should return
      """
      {
        "custodians": [],
        "parkrates": [],
        "motions": ["2b66fd261ee5c6cfc8de7fa466bab600bcfe4f69"],
        "fees": {}
      }
      """
    And motion "2b66fd261ee5c6cfc8de7fa466bab600bcfe4f69" should have 1 block
