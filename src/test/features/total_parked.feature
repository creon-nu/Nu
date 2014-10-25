Feature: The client keeps track of the total amount of NuBits parked
  Scenario: Total parked after parking
    Given a network with nodes "Alice" and "Bob" able to mint
    Then the total parked NuBits should reach "0"

    When node "Bob" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" votes a park rate of "0.001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until custodian "cust" is elected
    And node "Bob" reaches a balance of "1,000,000" NuBits
    And node "Bob" parks "50,000" NuBits for 4 blocks

    When node "Bob" finds a block
    Then the total parked NuBits should reach "50,000"

    When node "Bob" finds 4 blocks
    And node "Bob" unparks
    When node "Bob" finds a block
    Then the total parked NuBits should reach "0"
