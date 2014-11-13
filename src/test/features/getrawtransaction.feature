Feature: getrawtransaction returns data about known transactions

  Scenario: getrawtransaction on a normal block
    Given a network with nodes "Alice" and "Bob" able to mint
    And node "Bob" generates a NSR address "bob"
    When node "Alice" finds a block "A"
    And node "Alice" sends "1000" NSR to "bob"
    And node "Alice" finds a block "B" on top of "A"
    And the 1st transaction of block "B" on node "Alice" is named "coinbase"
    And the 2nd transaction of block "B" on node "Alice" is named "coinstake"
    And the 3rd transaction of block "B" on node "Alice" is named "tx"
    And all nodes reach block "B"
    Then getrawtransaction of "tx" should work on all nodes
    And getrawtransaction of "coinbase" should work on all nodes
    And getrawtransaction of "coinstake" should work on all nodes
