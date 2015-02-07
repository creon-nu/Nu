Feature: A command is called each time the vote changes. This can be used to update data feeds.
  Background:
    Given sample vote "alice vote" is:
      """
      {
         "custodians":[
            {"address":"bPwdoprYd3SRHqUCG5vCcEY68g8UfGC1d9", "amount":100.00000000},
            {"address":"bxmgMJVaniUDbtiMVC7g5RuSy46LTVCLBT", "amount":5.50000000}
         ],
         "parkrates":[
            {
               "unit":"B",
               "rates":[
                  {"blocks":8192, "rate":0.00030000},
                  {"blocks":16384, "rate":0.00060000},
                  {"blocks":32768, "rate":0.00130000}
               ]
            }
         ],
         "motions":[
            "8151325dcdbae9e0ff95f9f9658432dbedfdb209",
            "3f786850e387550fdab836ed7e6dc881de23001b"
         ],
         "fees": {
            "B": 0.05
         }
      }
      """

  Scenario: Votenotify is used as data feed source
    Given a node "Alice" started with a votenotify script
    And node "Alice" generates a NSR address "alice"
    And the votenotify script of node "Alice" is written to dump the vote and sign it with address "alice"
    And a data feed "Alice"
    And a node "Bob"
    When node "Alice" sets her vote to sample vote "alice vote"
    And the data feed "Alice" returns the content of the dumped vote and signature on node "Alice"
    And node "Bob" sets his data feed to the URL of "Alice" with address "alice"
    Then the vote of node "Bob" should be sample vote "alice vote"
