[![progress-banner](https://backend.codecrafters.io/progress/sqlite/b5df31dd-03cf-4a85-9ba3-9676008586bd)](https://app.codecrafters.io/users/kenpegrasio?r=2qF)

This is my C++ solutions to the
["Build Your Own SQLite" Challenge](https://codecrafters.io/challenges/sqlite).

# Setup Instruction (WSL)

## 1. Install `vcpkg`

```bash
wget -qO vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz
sudo mkdir /opt/vcpkg
sudo tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg
sudo /opt/vcpkg/bootstrap-vcpkg.sh
sudo ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg
rm -rf vcpkg.tar.gz
```

## 2. Set `VCPKG_ROOT` in the environment

```bash
echo 'export VCPKG_ROOT=/opt/vcpkg' | sudo tee -a /etc/profile.d/vcpkg.sh
source /etc/profile.d/vcpkg.sh
sudo chown -R $USER:$USER /opt/vcpkg
```

## 3. Convert .sh to Unix line endings

If `dos2unix` is not installed, run this following command
```bash
sudo apt update
sudo apt install -y dos2unix
```
Afterwards, run this command!
```bash
dos2unix your_program.sh
```

## 4. Install essential package 

```bash
sudo apt update
sudo apt install -y build-essential
```

## 5. Run

```bash
./your_program.sh sample.db .dbinfo
```

# Passing the first stage

The entry point for your SQLite implementation is in `src/Server.cpp`. Study and
uncomment the relevant code, and push your changes to pass the first stage:

```sh
git commit -am "pass 1st stage" # any msg
git push origin master
```

Time to move on to the next stage!

# Stage 2 & beyond

Note: This section is for stages 2 and beyond.

1. Ensure you have `cmake` installed locally
1. Run `./your_program.sh` to run your program, which is implemented in
   `src/Server.cpp`.
1. Commit your changes and run `git push origin master` to submit your solution
   to CodeCrafters. Test output will be streamed to your terminal.

# Sample Databases

To make it easy to test queries locally, we've added a sample database in the
root of this repository: `sample.db`.

This contains two tables: `apples` & `oranges`. You can use this to test your
implementation for the first 6 stages.

You can explore this database by running queries against it like this:

```sh
$ sqlite3 sample.db "select id, name from apples"
1|Granny Smith
2|Fuji
3|Honeycrisp
4|Golden Delicious
```

There are two other databases that you can use:

1. `superheroes.db`:
   - This is a small version of the test database used in the table-scan stage.
   - It contains one table: `superheroes`.
   - It is ~1MB in size.
1. `companies.db`:
   - This is a small version of the test database used in the index-scan stage.
   - It contains one table: `companies`, and one index: `idx_companies_country`
   - It is ~7MB in size.

These aren't included in the repository because they're large in size. You can
download them by running this script:

```sh
./download_sample_databases.sh
```

If the script doesn't work for some reason, you can download the databases
directly from
[codecrafters-io/sample-sqlite-databases](https://github.com/codecrafters-io/sample-sqlite-databases).
