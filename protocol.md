# ICT374

Client                                     Server
   |                                          |
   |------------ TCP Connection ------------->|
   |                                          |
   |<----------- RESPONSE_OK ("Username:") ----|
   |                                          |
   |---- RESPONSE_OK (<username>) ----------->|
   |                                          |
   |<----------- RESPONSE_OK ("Password:") ----|
   |                                          |
   |---- RESPONSE_OK (<password>) ----------->|
   |                                          |
   |<------ AUTH_SUCCESS / AUTH_FAIL ---------|
   |                                          |
   |     (If AUTH_SUCCESS, proceed to data relay)
   |                                          |
   |<============ Data Relay Phase ==========>|
   |                                          |

## x.x. Status Codes

#### The following status codes are defined

- 0 - RESPONSE_OK:          General success message.
- 1 - RESPONSE_FAIL:        General failure message.
- 2 - AUTH_SUCCESS:         Authentication successful.
- 3 - AUTH_FAIL:            Authentication failed.
- 4 - CONNECTION_SUCCESS:   Connection established successfully.
- 5 - CONNECTION_FAILURE:   Connection failed.
- 6 - COMMAND_SUCCESS:      Command executed successfully.
- 7 - COMMAND_FAIL:         Command execution failed.
- 8 - MESSAGE_TOO_LONG:     Message content too long.


## x.x. Message Format

#### Each message consists of

- Status Code       (1 byte)
- Content Length    (2 bytes, big-endian)
- Content           (variable length, up to 512 bytes)

## Message Format

- Status Code       (1 byte)
- Control Code      (1 byte)
- Content Length    (2 bytes, big-endian)
- Content           (variable length, up to 512 bytes)

## Control Codes

- 0 - NONE
- 1 - UP_ARROW
- 2 - DOWN_ARROW
- 3 - LEFT_ARROW
- 4 - RIGHT_ARROW
- 5 - RED
- 6 - GREEN