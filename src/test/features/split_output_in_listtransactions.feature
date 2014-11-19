Feature: Split outputs appear as a single transaction in listtransactions
  Scenario:
    Given a network with node "Alice" able to mint
    And a node "Bob" with an empty wallet
    And node "Bob" generates a new address "bob"
    When node "Alice" sends "30,000" NSR to "bob"
    Then all nodes should have 1 transaction in memory pool
    And node "Bob" should have 1 NSR transaction
    And the 1st transaction should be a receive of "30,000" to "bob"

    When node "Alice" finds a block received by all other nodes
    And node "Alice" generates a new address "alice"
    And node "Bob" sends "25,000" NSR to "alice"
    Then node "Bob" should have 2 NSR transaction
    And the 2nd transaction should be a send of "25,000" to "alice"
