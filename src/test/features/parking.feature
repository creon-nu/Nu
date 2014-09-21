Feature: NuBits can be parked

  Scenario: NuBits are parked and unparked
    Given a network with nodes "Alice" and "Bob" able to mint

    When node "Bob" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until custodian "cust" is elected
    And node "Bob" reaches a balance of "1,000,000" NuBits
    And node "Bob" parks "50,000" NuBits for 4 blocks
    Then node "Bob" should have a balance of "949,999.99" NuBits
    And "Bob" should have "50,000" NuBits parked
    And all nodes should have 1 transaction in memory pool

    When node "Alice" finds 3 blocks
    And all nodes reach the same height
    And node "Bob" unparks
    Then node "Bob" should have a balance of "949,999.99" NuBits
    And "Bob" should have "50,000" NuBits parked

    When node "Alice" finds a block
    And all nodes reach the same height
    Then node "Bob" should have a balance of "949,999.99" NuBits
    And "Bob" should have "50,000" NuBits parked

    When node "Bob" unparks
    Then node "Bob" should have a balance of "999,999.9905" NuBits
    And "Bob" should have "0" NuBits parked
