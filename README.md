wpantund, Userspace WPAN Network Daemon
=======================================

`wpantund` is a user-space network interface driver/daemon that
provides a native IPv6 network interface to a low-power wireless
**Network Co-Processor** (or *NCP*). It was written and developed by
Nest Labs to make supporting [Thread](http://threadgroup.org)
connectivity on unix-like operating systems more straightforward.

`wpantund` is designed to marshall all access to the NCP, ensuring
that it always remains in a consistent, well-defined state.

This is not an official Google product.

## Feature and Architecture Summary ##

`wpantund` provides:

 *  ...a native IPv6 interface to an NCP.
 *  ...a command line interface (`wpanctl`) for managing and
    configuring the NCP.
 *  ...a DBus API for managing and configuring the NCP.
 *  ...a way to reliably manage the power state of the NCP.
 *  ...a uniform mechanism for handling NCP firmware updates.

The architecture and design of `wpantund` has been motivated by the
following design goals (in no particular order):

 *  Portability across unix-like operating systems (Currently supports
    Linux and OS X. BSD support should be fairly trivial to add)
 *  Require few runtime dependencies (DBus, with boost needed when
    building)
 *  Single-threaded architecture, with heavy use of asynchronous I/O
 *  Power efficiency (0% CPU usage when idle)
 *  Allow management interface to be used by multiple independent
    applications simultaneously.
 *  Allow multiple instances of `wpantund` to gracefully co-exist on a
    single machine.
 *  Modular, plugin-based architecture (All details for communicating
    with a specific NCP stack are implemented as plugins)

Currently, the following NCP plugins are included:

*   `src/ncp-spinel`: Supports NCPs that communicate using the [Spinel NCP
    Protocol][1], used by NCPs running [OpenThread][2].
*   `src/ncp-dummy`: A dummy NCP plug-in implementation meant to be the
    starting point for implementing new NCP plug-ins.

[1]: ./third_party/openthread/src/ncp/PROTOCOL.md
[2]: https://github.com/openthread/openthread/

## License ##

`wpantund` is open-source software, released under the [Apache License,
Version 2.0][3]. See the file [`LICENSE`][4] for more information.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the [License][4] for the specific language governing permissions and
limitations under the License.

[3]: http://www.apache.org/licenses/LICENSE-2.0
[4]: ./LICENSE

## Conceptual Overview ##

`wpantund` is conceptually similar in purpose to the point-to-point
daemon (`pppd`, commonly used on unix platforms to provide network
connectivity via a dial-up modems)---except that instead of
communicating with a dial-up modem, `wpantund` is communicating with
an NCP.

`wpantund` communicates with the NCP via an abstraction of a
asynchronous stream socket, which could be any of the following:

 *  A real serial port (UART) connected to the NCP (preferably with
    hardware flow control)
 *  The stdin and stdout from a subprocess (For supporting SPI
    interfaces using a translator program, or debugging virtual
    stacks)
 *  A TCP socket (For debugging, not recommended for production)

Unlike a dial-up modem, NCPs often have a rich management interface
for performing operations like forming a network, joining a network,
scanning for nearby networks, etc. To perform these operations,
`wpantund` includes a command line utility called `wpanctl`.
Applications that need to directly configure the network interface can
also communicate directly with `wpantund` using its DBus API.

To expose a native IPv6 network interface to the host operating
system, `wpantund` uses the `tun` driver on Linux and the `utun`
driver on OS X. On Linux, the default name for this interface is
`wpan0`. On OS X, the default name is `utun0`.

## Usage Overview ##

The behavior of `wpantund` is determined by its configuration
parameters, which may be specified in a configuration file (typically
`/etc/wpantund.conf`) or at the command line. A typical configuration
file might look like this: (For a more thorough explanation of
available configuration parameters, see the [included example][5]).

    # Try to name the network interface `wpan0`.
    # If not possible, a different name will be used.
    Config:TUN:InterfaceName      "wpan0"

    # The pathname of the socket used to communicate
    # with the NCP.
    Config:NCP:SocketPath         "/dev/ttyUSB0"

    # The name of the driver plugin to use. The chosen
    # plugin must support the NCP you are trying to use.
    Config:NCP:DriverName         "spinel"

    # Drop root privileges after opening all sockets
    Config:Daemon:PrivDropToUser  "nobody"

    # Use a CCA Threshold of -70db
    NCP:CCAThreshold              "-70"

When up and running, `wpanctl` can be used to check the status of the
interface and perform various management operations. For example, to
check the general status of an interface:

    $ sudo wpanctl status
    wpan0 => [
        "NCP:State" => "offline"
        "Daemon:Enabled" => true
        "NCP:Version" => "OPENTHREAD/g1651a47; May 23 2016 17:23:24"
        "Daemon:Version" => "0.07.00 (May 23 2016 12:58:54)"
        "Config:NCP:DriverName" => "spinel"
        "NCP:HardwareAddress" => [F1D92A82C8D8FE43]
    ]

Here we see that the NCP is in the `offline` state, along with a few
additional bits of information like the version of the NCP and it's
hardware address. From here we can easily form a new network:

    $ sudo wpanctl form "wpantund-testnet"
    Forming WPAN "wpantund-testnet" as node type router
    Successfully formed!
    $

And now if we check the status, we will see a lot more information:

    $ sudo wpanctl status
    wpan0 => [
        "NCP:State" => "associated"
        "Daemon:Enabled" => true
        "NCP:Version" => "OPENTHREAD/g1651a47; May 23 2016 17:23:24"
        "Daemon:Version" => "0.07.00 (May 23 2016 12:58:54)"
        "Config:NCP:DriverName" => "spinel"
        "NCP:HardwareAddress" => [F1D92A82C8D8FE43]
        "NCP:Channel" => 23
        "Network:NodeType" => "leader"
        "Network:Name" => "wpantund-testnet"
        "Network:XPANID" => 0x09717AEF221F66FB
        "Network:PANID" => 0xBFCD
        "IPv6:LinkLocalAddress" => "fe80::f3d9:2a82:c8d8:fe43"
        "IPv6:MeshLocalAddress" => "fd09:717a:ef22::9a5d:5d1e:5527:5fc8"
        "IPv6:MeshLocalPrefix" => "fd09:717a:ef22::/64"
    ]
    $ ifconfig wpan0
    wpan0: flags=8051<UP,POINTOPOINT,RUNNING,MULTICAST> mtu 1280
        inet6 fe80::f3d9:2a82:c8d8:fe43%wpan0 prefixlen 10 scopeid 0x15
        inet6 fd09:717a:ef22::9a5d:5d1e:5527:5fc8 prefixlen 64

If compiled with `libreadline` or `libedit`, `wpanctl` supports an
interactive console, which is very convenient. All commands support
online help: simply type `help` to get a list of supported commands,
or add `-h` to a command to get help with that specific command.

See the wiki for more information: <https://github.com/openthread/wpantund/wiki>

[5]: ./src/wpantund/wpantund.conf

## Support ##

Submit bugs and feature requests to [issue tracker][6]. We use the
following mailing lists for discussion and announcements:

 *  [wpantund-announce](https://groups.google.com/forum/#!forum/wpantund-announce)
    \- Official Anouncements About `wpantund`
 *  [wpantund-users](https://groups.google.com/forum/#!forum/wpantund-users)
    \- `wpantund` User Discussion Group
 *  [wpantund-devel](https://groups.google.com/forum/#!forum/wpantund-devel)
    \- `wpantund` Developer Discussion Group

[6]: https://github.com/openthread/wpantund/issues

## Authors and Contributors ##

The following people have significantly contributed to the design
and development of `wpantund`:

 *  Robert Quattlebaum
 *  Marcin Szczodrak
 *  Vaas Krishnamurthy
 *  Arjuna Siva
 *  Abtin Keshavarzian

If you would like to contribute to this project, please read
[CONTRIBUTING.md](CONTRIBUTING.md) first.
