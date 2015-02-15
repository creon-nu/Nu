Feature: NuBits can be parked

  Scenario: NuBits are parked and unparked
    Given a network with nodes "Alice" and "Bob" able to mint

    When node "Bob" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until custodian "cust" is elected
    And node "Bob" reaches a balance of "1,000,000" NuBits
    And node "Bob" parks "50,000" NuBits for 4 blocks with "cust" as unpark address
    Then node "Bob" should have a balance of "949,999.99" NuBits
    And "Bob" should have "50,000" NuBits parked
    And node "Bob" should have 2 NuBits transactions
    And the 2nd transaction should be a send of "50,000" to "cust"
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
    And node "Bob" should have 3 NuBits transactions
    And the 3rd transaction should be a receive of "50,000.0005" to "cust"
    Then node "Bob" should have a balance of "999,999.9905" NuBits
    And "Bob" should have "0" NuBits parked

  Scenario: A client running 0.4.2 accepts an unpark transaction in a block
    Given a network with nodes "Alice" and "Bob" able to mint
    And a node "Old" running version "0.4.2"

    When node "Alice" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until custodian "cust" is elected
    And node "Alice" reaches a balance of "1,000,000" NuBits
    And node "Alice" parks "50,000" NuBits for 4 blocks
    And node "Alice" finds 4 blocks
    And all nodes reach the same height
    Then "Alice" should have "50,000" NuBits parked

    When node "Alice" unparks to transaction "unpark"
    And node "Alice" finds a block "X"
    Then block "X" should contain transaction "unpark"
    And all nodes should be at block "X"

  Scenario: Parking with an unpark address not in your wallet
    Given a network with nodes "Alice" and "Bob" able to mint
    And node "Alice" generates a NuBit address "alice"
    And node "Alice" votes an amount of "1,000,000" for custodian "alice"
    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until custodian "alice" is elected
    And node "Alice" reaches a balance of "1,000,000" NuBits
    And node "Bob" generates a NuBit address "bob"

    When node "Alice" parks "50,000" NuBits for 4 blocks with "bob" as unpark address
    And node "Alice" finds a block received by all other nodes

    Then node "Alice" should have "0" NuBits parked
    And node "Alice" should have 2 NuBits transactions
    And the 2nd transaction should be a send of "50,000" to "bob"

    And node "Bob" should have "50,000" NuBits parked
    And node "Bob" should have 0 NuBits transactions

    When node "Alice" finds 4 blocks received by all other nodes
    And node "Bob" unparks
    And node "Bob" finds a block received by all other nodes

    Then node "Alice" should have "0" NuBits parked
    And node "Alice" should have 2 NuBits transactions

    And node "Bob" should have "0" NuBits parked
    And node "Bob" should have 1 NuBits transactions
    And the transaction should be a receive of "50,000.0005" to "bob"

  Scenario: Unparking with not enough amount sent to a new node
    Given a network with nodes "Alice" and "Bob" able to mint
    And node "Alice" grants herself "1,000,000" NuBits

    And a node "Old" running version "0.5.2"
    And node "Alice" sends "100,000" NSR to node "Old"
    And node "Alice" finds blocks until just received NSR are able to mint

    And a node "Unparker" only connected to node "Alice"

    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until voted park rate becomes effective

    And node "Alice" parks "50,000" NuBits for 4 blocks
    And node "Alice" finds 4 blocks received by all other nodes

    When node "Unparker" unparks the last park of node "Alice" with an amount of "0.01" NBT
    Then node "Alice" should stay at 0 transactions in memory pool
    And node "Old" should stay at 0 transactions in memory pool

  Scenario: Unparking with not enough amount sent to an old node
    Given a network with nodes "Alice" and "Bob" able to mint
    And node "Alice" grants herself "1,000,000" NuBits

    And a node "Old" running version "0.5.2"
    And node "Alice" sends "500,000" NSR to node "Old"
    And node "Alice" finds blocks until just received NSR are able to mint

    And a node "Unparker" only connected to node "Old"

    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until voted park rate becomes effective

    And node "Alice" parks "50,000" NuBits for 4 blocks
    And node "Alice" finds 4 blocks received by all other nodes

    When node "Unparker" unparks the last park of node "Alice" with an amount of "0.01" NBT
    Then node "Old" should reach 1 transaction in memory pool
    And node "Alice" should stay at 0 transactions in memory pool

    When node "Alice" finds a block "X"
    And node "Old" reaches block "X"
    And node "Old" finds a block
    Then node "Alice" should stay at block "X"
