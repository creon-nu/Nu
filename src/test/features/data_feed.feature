Feature: The user can define a data feed URL to automatically update his vote from an external source
  Scenario: An user sets a data feed
    Given a network with node "Alice" able to mint
    And a data feed "Bob"
    And the data feed "Bob" returns:
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
         ]
      }
      """
    When node "Alice" sets her data feed to the URL of "Bob"
    Then the vote of node "Alice" should be:
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
         ]
      }
      """

  Scenario: The data feed updates
    Given a network with node "Alice" able to mint
    And a data feed "Bob"
    And the data feed "Bob" returns:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[]
      }
      """
    When node "Alice" sets her data feed to the URL of "Bob"
    And the vote of node "Alice" is:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[]
      }
      """
    And the data feed "Bob" returns:
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
         ]
      }
      """
    And 60 seconds pass
    Then the vote of node "Alice" should become:
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
         ]
      }
      """

  Scenario: A data feed is set and the client is restarted
    Given a network with node "Alice" able to mint
    And a data feed "Bob"
    When node "Alice" sets her data feed to the URL of "Bob"
    And node "Alice" restarts
    Then node "Alice" should use the data feed "Bob"

  Scenario: A data feed sends too many data
    Given a network with node "Alice" able to mint
    And a data feed "Bob"
    And the data feed "Bob" returns a valid vote of 10241 bytes
    When node "Alice" sets her data feed to the URL of "Bob"
    And the vote of node "Alice" is:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[]
      }
      """
    And the error on node "Alice" should be "Data feed size exceeds limit (10240 bytes)"

  Scenario: A data feed sends incomplete vote
    Given a network with node "Alice" able to mint
    And a data feed "Bob"
    And node "Alice" sets her vote to:
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
         ]
      }
      """
    And the data feed "Bob" returns:
      """
      {
         "motions":[
            "1111111111111111111111111111111111111111"
         ]
      }
      """
    When node "Alice" sets her data feed to the URL of "Bob"
    Then the vote of node "Alice" should be:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[
            "1111111111111111111111111111111111111111"
         ]
      }
      """

  Scenario: A data feed is down
    Given a network with node "Alice" able to mint
    And a data feed "Bob"
    And data feed "Bob" shuts down
    When node "Alice" sets her data feed to the URL of "Bob"
    Then the vote of node "Alice" should be:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[]
      }
      """
      And the error on node "Alice" should be "Data feed failed: Couldn't connect to server"

  Scenario: A data feed returns invalid JSON
    Given a network with node "Alice" able to mint
    And node "Alice" sets her vote to:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[
            "1111111111111111111111111111111111111111"
         ]
      }
      """
    And a data feed "Bob"
    And the data feed "Bob" returns:
      """
      <html><body>Hello</body></html>
      """
    When node "Alice" sets her data feed to the URL of "Bob"
    Then the vote of node "Alice" should be:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[
            "1111111111111111111111111111111111111111"
         ]
      }
      """
      And the error on node "Alice" should be "Data feed returned invalid data"

  Scenario: A data feed returns non 200 code
    Given a network with node "Alice" able to mint
    And node "Alice" sets her vote to:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[
            "1111111111111111111111111111111111111111"
         ]
      }
      """
    And a data feed "Bob"
    And the data feed "Bob" returns a status 500 with the body:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[
            "2222222222222222222222222222222222222222"
         ]
      }
      """
    When node "Alice" sets her data feed to the URL of "Bob"
    Then the vote of node "Alice" should be:
      """
      {
         "custodians":[],
         "parkrates":[],
         "motions":[
            "1111111111111111111111111111111111111111"
         ]
      }
      """
    And the error on node "Alice" should be "Data feed failed: Response code 500"
