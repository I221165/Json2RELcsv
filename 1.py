#!/usr/bin/env python3
import sys, os, json, csv

def walk(node, tablename, parent_id, seq_index, mgr):
    """ Recursively walk the JSON node into tables. """
    if isinstance(node, dict):
        # start a new row
        row_id = mgr[tablename]["_next_id"]
        mgr[tablename]["_next_id"] += 1
        row = {"id": str(row_id)}
        if parent_id is not None:
            row[f"{tablename}_id"] = str(parent_id)
        if seq_index is not None:
            row["seq"] = str(seq_index)

        # separate scalars vs nested
        for k, v in node.items():
            if isinstance(v, (dict, list)):
                walk(v, k, row_id, None, mgr)
            else:
                row[k] = "" if v is None else str(v)
        mgr[tablename]["rows"].append(row)

    elif isinstance(node, list):
        for idx, item in enumerate(node):
            if isinstance(item, (dict, list)):
                walk(item, tablename, parent_id, idx, mgr)
            else:
                # simple-array: table=tablename, with index+value
                row_id = mgr[tablename]["_next_id"]
                mgr[tablename]["_next_id"] += 1
                mgr[tablename]["rows"].append({
                    "id": str(row_id),
                    f"{tablename}_id": str(parent_id) if parent_id is not None else "",
                    "seq": str(idx),
                    "value": "" if item is None else str(item)
                })

def write_csvs(mgr, outdir):
    os.makedirs(outdir, exist_ok=True)
    for table, data in mgr.items():
        # collect all column names in order of first appearance
        cols = ["id"] + [c for c in data["cols_order"]
                         if c not in ("id",)]
        # also include f"{table}_id" and "seq" if used
        if any(f"{table}_id" in r for r in data["rows"]):
            cols.append(f"{table}_id")
        if any("seq" in r for r in data["rows"]):
            cols.append("seq")
        # write
        path = os.path.join(outdir, f"{table}.csv")
        with open(path, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=cols, extrasaction="ignore")
            writer.writeheader()
            writer.writerows(data["rows"])
        print(f"Wrote {path}")

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} OUT_DIR INPUT.json")
        sys.exit(1)

    outdir = sys.argv[1]
    infile = sys.argv[2]
    with open(infile) as f:
        data = json.load(f)

    # manager: for each table name, store list of rows and next id
    mgr = {}
    def ensure_table(name):
        if name not in mgr:
            mgr[name] = {"rows": [], "_next_id": 1, "cols_order": []}
        return mgr[name]

    # start at root; top-level table named “root” (or basename)
    root_table = os.path.splitext(os.path.basename(infile))[0] or "root"
    ensure_table(root_table)
    walk(data, root_table, None, None, mgr)

    # discover columns from all rows (in insertion order)
    for table, d in mgr.items():
        seen = set()
        for row in d["rows"]:
            for k in row:
                if k not in seen:
                    seen.add(k)
                    d["cols_order"].append(k)

    write_csvs(mgr, outdir)

if __name__ == "__main__":
    main()
