Feature: NuShare change is part of the keys exported to Peercoin
  Scenario: After shares are sent, the total outputs of the exported keys include the change
    Given a network with nodes "Alice" and "Bob" able to mint
    When node "Bob" generates a NuShare address "bob"
    And node "Alice" sends "1,000" NuShares to "bob"
    And node "Alice" reaches a balance of "9,998,999" NuShares
    Then the total unspent amount of all the Peercoin keys on node "Alice" should be "9,998,999"

  Scenario: Sending NuShares generates a single transaction
    Given a network with nodes "Alice" and "Bob" able to mint
    And node "Bob" generates a NuShares address "bob"
    And node "Alice" sends "1000" NuShares to "bob"
    Then node "Alice" should have 2 NuShares transactions
    And the 1st transaction should be the initial distribution of shares
    And the 2nd transaction should be a send of "1000" to "bob"

    When node "Alice" finds a block received by all other nodes
    Then node "Bob" should have 2 NuShare transactions
    And the 1st transaction should be the initial distribution of shares
    And the 2nd transaction should be a receive of "1000" to "bob"
