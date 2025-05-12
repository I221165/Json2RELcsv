
This tool reads a JSON file and writes one or more relational CSV files according to the following rules:

1. **Top-level objects** become rows in a table named after the file (or `root` if no name).
2. **Nested objects/arrays** become additional tables, with foreign-key back-references.
3. Scalars (`string`, `number`, `bool`, `null`) become columns in the table.
4. Each row gets an `id`; child rows get a `<parent>_id`; array items also get a `seq` index.
5. `null` values are emitted as empty fields. Strings containing commas or newlines are quoted.

---


**Required argument**  
- `<input.json>`  
  Path to the JSON file you want to convert. Must be the last argument.

**Options**  
- `--out-dir DIR`  
  Write all generated `.csv` files into directory `DIR`.  
  - If `DIR` does not exist, it will be created.  
  - Default: the current working directory (`.`).

- `--print-ast`  
  After parsing, print the parsed Abstract Syntax Tree (AST) in an indented, human‐readable form, then proceed to CSV generation. Useful for debugging or understanding how the JSON is being interpreted.

- `-h`, `--help`  
  Display a brief usage message and exit.

**Behavior**  
1. **Parsing**  
   - Reads and tokenizes the JSON file.  
   - On any **lexical** error (e.g. unterminated string), prints  
     ```
     Error lexing at LINE:COL: <message>
     ```  
     and exits with status 1.  
   - On any **syntax** error (e.g. missing comma), prints  
     ```
     Error parsing at LINE:COL: <message>
     ```  
     and exits with status 1.

2. **AST printing** (if `--print-ast`):  
   - Dumps the entire JSON structure as an indented tree:  
     ```
     Object {
       users:
         Array [
           Object { id: Number 1; name: String "Alice" }
           Object { id: Number 2; name: String "Bob" }
         ]
     }
     ```

3. **Schema & CSV generation**  
   - Walks the AST to detect tables, assign primary keys (`id`), foreign keys (`<parent>_id`), and sequence indices (`seq`).  
   - Writes one `.csv` file per table into `--out-dir`. Filenames are `<table>.csv`.  
   - Strings containing commas or newlines are enclosed in double quotes.  
   - `null` values appear as empty cells.  

**Examples**

1. **Basic conversion**  
   ```bash
   # Convert example1.json → out/example1.csv
   mkdir -p out
   ./json2relcsv --out-dir out example1.json


## Build

```bash
sudo apt update
sudo apt install -y flex bison build-essential
make clean
make

mkdir -p out
#./json2relcsv --out-dir out [--print-ast] input.json    this is an example of usage
./json2relcsv --out-dir out example1.json

