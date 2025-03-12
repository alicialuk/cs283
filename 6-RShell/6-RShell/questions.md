1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

	The remote client determines when a command’s output is fully received by looking for the EOF character 0x04 at the end of the message. To handle partial reads or ensure complete message transmission, the client uses “%.*s” format specifier to display the incomplete message chunks correctly.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A networked shell protocol should define and detect the beginning and end of a command sent over a TCP connection by implementing application-level message framing. If this is not handled correctly messages could be merged incorrectly which can lead to command misinterpretation. 

3. Describe the general differences between stateful and stateless protocols.

The general difference is that stateful protocols maintain client session information between requests and requires more server resources. While, stateless protocols treat each request independently with no session memory.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

TCP since it eliminates connection establishment, acknowledgments, and ordering overhead. 

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Operating systems provide Socket API as the primary abstraction for network communications. This allows applications to establish connection and transfer data using standard function calls.
