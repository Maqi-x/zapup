# ZapUp
ZapUp is a command-line version manager for the Zap Programming Language. It allows you to install, manage, and switch between different Zap versions easily.

## Installation

### Quick Install

Run the following command:

```sh
sh -c "$(curl -sS https://raw.githubusercontent.com/thezaplang/zapup/refs/heads/main/quick-install.sh)"
```

This script will:

* clone the repository into a temporary directory
* download dependencies
* build the project
* install it using `sudo make install`

> [!WARNING]
> This command downloads and executes a remote script.
> You can inspect it here before running:
> https://github.com/thezaplang/zapup/blob/main/quick-install.sh

---

### Arch User Repository (AUR)

ZapUp is also available in the Arch User Repository (AUR). You can install it using an AUR helper like `yay`:

```sh
yay -S zapup
```

or `paru`:
```sh
paru -S zapup
```

---

### Manual installation

Clone the zapup repo:
```sh
git clone https://github.com/thezaplang/zapup.git
cd zapup
```

Download dependencies:
```sh
make submodules
```

Build the project:
```sh
make -j$(nproc)
```

Install (optional):
```sh
sudo make install
```

---

## Usage
Run `zapup help` for usage information.

---

## License
This project is licensed under the **GNU GPL v3 License** - see the [LICENSE](https://github.com/thezaplang/ZapUp/blob/main/LICENSE) file for details or obtain a copy [here](https://www.gnu.org/licenses/gpl-3.0.en.html).
