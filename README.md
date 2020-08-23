# UC Powered Up #

This project is a C implementation of the LEGO Powered Up Bluetooth
Low-Energy (BLE) protocol which can be used to control LEGO Powered
Up Hubs. The project has minimal dependencies (only libbluetooth),
and the communication is easy to follow with a debugger since message
processing is synchronous. Furthermore comments are added to explain
the tricky parts so the project is useful to learn about the basics
of BLE communication.

The LEGO Hub uses the Bluetooth Attribute Protocol (ATT) for controlling
the Hub. This protocol defines how attributes can be enumerated, read,
or written, and how clients can receive notifications about attribute
value changes. Normally attributes are used for accessing server data
and notifications are used to inform the client about the new value of
an attribute. As for LEGO Hubs, attribute writing is used for issuing
commands, and notifications are used for receiving various messages
from the Hub. The reason of using attribute protocol for communication
is that it has a reserved channel id (its value is 4) and its message
format is simple so the protocol overhead is acceptable (around three
bytes are wasted for each message).

The detailed LEGO Generic Attribute Protocol (GATT) can be found here:

https://lego.github.io/lego-ble-wireless-protocol-docs/

Due to the synchronous nature of the library, it is easy to define the
necessary steps (stages) to control a given LEGO model. It is recommended
to enable only those notifications which are required by the model (i.e.
when built-in tilt sensor is not needed, it is better to keep it disabled).
Processing these notifications can be postponed and unwanted notifications
can be discarded easily.

The project is in its early phases. Any contributions are welcome.
