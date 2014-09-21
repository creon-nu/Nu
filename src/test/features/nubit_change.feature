Feature: NuBit change is not reported as a transaction

  Scenario: Sending NuBits generates a single transaction
    Given a network with nodes "Alice", "Bob" and "Custodian" able to mint
    When node "Custodian" generates a "NuBit" address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" finds blocks until custodian "cust" is elected
    And node "Bob" generates a "NuBit" address "bob"
    And all nodes reach the same height
    And some time pass
    And node "Custodian" sends "1000" NuBits to "bob"
    Then node "Custodian" should have 2 NuBit transactions
    And the 2nd transaction sould be a send of "1000" to "bob"
    And node "Bob" should reach an unconfirmed balance of "1000" NuBits
    And node "Bob" should have 1 NuBit transactions
    And the 1st transaction sould be a receive of "1000" to "bob"
