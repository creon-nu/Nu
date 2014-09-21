Feature: NuShare change is part of the keys exported to Peercoin
  Scenario: After shares are sent, the total outputs of the exported keys include the change
    Given a network with nodes "Alice" and "Bob" able to mint
    When node "Bob" generates a NuShare address "bob"
    And node "Alice" sends "1,000" NuShares to "bob"
    And node "Alice" reaches a balance of "9,998,999" NuShares
    Then the total unspent amount of all the Peercoin keys on node "Alice" should be "9,998,999"
