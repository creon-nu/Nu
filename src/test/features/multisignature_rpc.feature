Feature: Ability to sign multisignature transaction without all the keys
  Scenario: Listing unspent outputs of a multisignature address
    Given a network with nodes "Alice", "Bob"

    When node "Bob" generates a new address "a"
    And node "Bob" generates a new address "b"
    And the public key of address "a" is retreived from node "Bob"
    And the public key of address "b" is retreived from node "Bob"

    And node "Bob" adds a multisig address "multi" requiring 2 keys from the public keys "a" and "b"
    And node "Alice" sends "5,000" to "multi"
    Then node "Bob" should reach an unspent amount of "5,000" on address "multi"

  Scenario: Two nodes send coins from a multisignature output
    Given a network with nodes "Alice", "Bob", "Carol", "Dan" and "Erin"

    When node "Alice" generates a new address "alice"
    And the public key of address "alice" is retreived from node "Alice"

    And node "Bob" generates a new address "bob"
    And the public key of address "bob" is retreived from node "Bob"

    And node "Carol" creates a multisig address "multi" requiring 2 keys from the public keys "alice" and "bob"
    And node "Carol" sends "5,000" to "multi" in transaction "tx"
    And node "Carol" finds a block received by all other nodes

    And node "Erin" generates a new address "recipient"
    And node "Dan" generates a new address "change"
    And node "Dan" generates a raw transaction "raw" to send the amount sent to address "multi" in transaction "tx" to:
      | Address   | Value    |
      | recipient | 1,000.00 |
      | change    | 3,999.00 |
    And node "Alice" adds a multisig address "multi" requiring 2 keys from the public keys "alice" and "bob"
    And node "Alice" signs the raw transaction "raw"
    And node "Bob" adds a multisig address "multi" requiring 2 keys from the public keys "alice" and "bob"
    And node "Bob" signs the raw transaction "raw"
    Then the raw transaction "raw" should be complete

    When node "Dan" sends the raw transaction "raw"
    Then all nodes should reach 1 transaction in memory pool

    When node "Alice" finds a block
    Then node "Erin" should reach a balance of "10,001,000"

  Scenario: Send to multisig nubits
