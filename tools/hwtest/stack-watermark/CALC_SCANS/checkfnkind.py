#!/usr/bin/env python3
"""Read a FNKIND run and compare it with a stored one.

A run leaves a dated .REGS.TSV beside the screen captures it took. This reads the pair, prints the
table, and either stores it as the reference for a machine or checks a later run against that
reference. Nothing here decides whether a figure is correct: it reports what moved and by how much.

    python3 checkfnkind.py show    <run folder>
    python3 checkfnkind.py compare <run folder> <run folder>

A run folder is wherever the .REGS.TSV and the .bmp files of one run were put. compare takes two of
them and returns 1 if anything moved. Nothing is stored anywhere: the runs themselves are the record.

The stack figure is compared in bytes and the time as a fraction of itself. The stack figure of a
deep case repeats exactly between runs of one build; a shallow one moves by up to about 140 bytes,
because there the figure is the depth of the measuring machinery and not of the case, and which word
of that frame the scan finds depends on what the step before left behind. The default of 160 bytes
is set above that. The time is not compared directly: each case runs for a fixed window and what is
compared is how many calls fitted in it, which is the same number on the same machine and needs no
sizing to be carried between machines. A case where fewer than twenty calls fitted is called out
rather than compared, since one call either way is then worth over five per cent.
"""

import argparse
import hashlib
import os
import re
import sys

DEFAULT_TIME_TOLERANCE = 0.20     # fraction the loop time may move before it is reported
DEFAULT_STACK_TOLERANCE = 160     # bytes the stack figure may move before it is reported, see below
CALL_FLOOR = 20                   # calls: below this, one call either way is worth over 5 per cent
STATUS_MEASURED = 0               # STCKST value that means the stack figure is a depth reached

STATUS_TEXT = {
    0: "measured",
    1: "marker ran out, floor only",
    2: "nothing disturbed",
    3: "no marker laid, stale",
    9: "the build has no stack tool, so nothing wrote over what KSET stored",
}

ROW = re.compile(r'^X\t"([^"]*)"\tY\t"([^"]*)"$')


def read_tsv(path):
    """The rows the program printed, as {label: text}. The stack blocks a capture appends have no
    quoted label and are skipped."""
    values = {}
    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            match = ROW.match(line.rstrip("\n"))
            if match:
                values[match.group(1)] = match.group(2)
    return values


def find_tsv(directory):
    """Every .REGS.TSV in the folder, oldest first by the name the calculator gave it. A run that a crash
    cut short is finished by a second run of the cases that were left, which writes a second file; the two
    together are the run. A case appearing twice takes the later reading."""
    names = sorted(name for name in os.listdir(directory) if name.endswith(".REGS.TSV"))
    if not names:
        raise SystemExit("no .REGS.TSV in %s" % directory)
    return [os.path.join(directory, name) for name in names]


def captures(directory):
    """Every screen capture the run took, in the order it took them, with its SHA-256."""
    out = []
    for name in sorted(name for name in os.listdir(directory) if name.lower().endswith(".bmp")):
        with open(os.path.join(directory, name), "rb") as handle:
            out.append({"name": name, "bytes": os.path.getsize(os.path.join(directory, name)),
                        "sha256": hashlib.sha256(handle.read()).hexdigest()})
    return out


def as_int(text):
    try:
        return int(text)
    except (TypeError, ValueError):
        return None


def collect(directory):
    """One run, as the table the rest of this works from."""
    values = {}
    for path in find_tsv(directory):
        values.update(read_tsv(path))
    cases = []
    for label, figure in values.items():
        match = re.match(r"^(\d\d) (.+)$", label)
        if not match:
            continue
        number, name = match.group(1), match.group(2)
        cases.append({
            "number": number,
            "name": name,
            "stack": as_int(figure),
            "status": as_int(values.get("s" + number)),
            "span": as_int(values.get("u" + number)),
            "calls": as_int(values.get("c" + number)),
            "tenths": as_int(values.get("e" + number)),
        })
    cases.sort(key=lambda case: case["number"])
    return {"cases": cases, "captures": captures(directory)}


def per_call_ms(case, overhead_ms):
    """Milliseconds one call of this case took, less the loop and call overhead case 01 measures."""
    if not case["calls"] or case["tenths"] is None:
        return None
    return case["tenths"] * 100.0 / case["calls"] - overhead_ms


def overhead(run):
    for case in run["cases"]:
        if case["number"] == "01":
            if case["calls"] and case["tenths"] is not None:
                return case["tenths"] * 100.0 / case["calls"]
    return 0.0


def show(run):
    base = overhead(run)
    print("%-3s %-10s %8s %-6s %8s %6s %10s %10s" %
          ("nr", "kind", "stack", "status", "span", "tenths", "calls", "ms/call"))
    for case in run["cases"]:
        call = per_call_ms(case, base)
        print("%-3s %-10s %8s %-6s %8s %6s %10s %10s" % (
            case["number"], case["name"],
            case["stack"] if case["stack"] is not None else "-",
            case["status"] if case["status"] is not None else "-",
            case["span"] if case["span"] is not None else "-",
            case["tenths"] if case["tenths"] is not None else "-",
            case["calls"] if case["calls"] is not None else "-",
            "-" if call is None else "%.3f" % call))
    print()
    print("loop and call overhead, from case 01: %.3f ms per call" % base)
    print("captures: %d" % len(run["captures"]))
    for capture in run["captures"]:
        print("  %s  %d bytes  %s" % (capture["name"], capture["bytes"], capture["sha256"][:16]))
    unmeasured = [case for case in run["cases"] if case["status"] != STATUS_MEASURED]
    for case in unmeasured:
        print("STACK FIGURE IS NOT A MEASUREMENT: %s %s, STCKST %s, %s"
              % (case["number"], case["name"], case["status"],
                 STATUS_TEXT.get(case["status"], "unknown code")))
    few = [case for case in run["cases"]
           if case["calls"] is not None and case["calls"] < CALL_FLOOR]
    if few:
        print("fewer than %d calls fitted the window, so one call either way is worth over 5 per cent."
              % CALL_FLOOR)
        print("A single call of these is at or past the window, which is a result in itself; lengthen the")
        print("window in the table only if you want the count resolved: %s"
              % ", ".join("%s %s (%s)" % (case["number"], case["name"], case["calls"]) for case in few))
    return 0


def compare(old, new, tolerance, stack_tolerance):
    """Two runs, side by side. Whichever folder you name first is the one being compared against."""
    problems = []
    was = {case["number"]: case for case in old["cases"]}
    now = {case["number"]: case for case in new["cases"]}

    for number in sorted(set(was) | set(now)):
        if number not in now:
            problems.append("case %s %s ran in the first and not in the second: it stopped before it"
                            % (number, was[number]["name"]))
            continue
        if number not in was:
            problems.append("case %s %s ran in the second and not in the first" % (number, now[number]["name"]))
            continue
        first, second = was[number], now[number]
        if first["name"] != second["name"]:
            problems.append("case %s is %s in the first and %s in the second"
                            % (number, first["name"], second["name"]))
        if second["status"] != STATUS_MEASURED:
            problems.append("case %s %s: STCKST %s, %s, so its stack figure is not a measurement"
                            % (number, second["name"], second["status"],
                               STATUS_TEXT.get(second["status"], "unknown code")))
        elif first["status"] == STATUS_MEASURED and abs(second["stack"] - first["stack"]) > stack_tolerance:
            problems.append("case %s %s: stack %s against %s, %+d bytes"
                            % (number, second["name"], second["stack"], first["stack"],
                               second["stack"] - first["stack"]))
        if first["calls"] and second["calls"] is not None:
            if first["calls"] >= CALL_FLOOR:
                moved = (second["calls"] - first["calls"]) / float(first["calls"])
                if abs(moved) > tolerance:
                    problems.append("case %s %s: %s calls against %s, %+.0f per cent"
                                    % (number, second["name"], second["calls"], first["calls"], moved * 100.0))

    if len(new["captures"]) != len(old["captures"]):
        problems.append("%d captures against %d" % (len(new["captures"]), len(old["captures"])))
    for index, (second, first) in enumerate(zip(new["captures"], old["captures"])):
        if second["sha256"] != first["sha256"]:
            problems.append("capture %d differs: %s against %s" % (index + 1, second["name"], first["name"]))

    for line in problems:
        print(line)
    if problems:
        print()
        print("%d differences" % len(problems))
        return 1
    print("no differences: %d cases and %d captures" % (len(new["cases"]), len(new["captures"])))
    return 0


def is_run(directory):
    """A run folder is one holding exactly the .REGS.TSV a run wrote."""
    try:
        return any(name.endswith(".REGS.TSV") for name in os.listdir(directory))
    except OSError:
        return False


def all_runs():
    """Every run folder under this script's own folder, oldest first by the name the calculator gave the
    TSV, which is its clock. The cwd counts too, so the script works from inside a run folder."""
    root = os.path.dirname(os.path.abspath(__file__))
    found = {}
    for where, directories, names in os.walk(root):
        directories[:] = [d for d in directories if not d.startswith(".")]
        stamps = sorted(n for n in names if n.endswith(".REGS.TSV"))
        if stamps:
            found[os.path.abspath(where)] = stamps[0]
    if is_run(os.getcwd()):
        here = os.path.abspath(os.getcwd())
        found.setdefault(here, sorted(n for n in os.listdir(here) if n.endswith(".REGS.TSV"))[0])
    return [path for path, _stamp in sorted(found.items(), key=lambda item: (item[1], item[0]))]


def resolve(given, runs):
    """A run folder from whatever the user typed: a path from here, a path from the script's folder, or
    any part of a run's name. Anything else lists what there is instead of a stack trace."""
    if given:
        for candidate in (given, os.path.join(os.path.dirname(os.path.abspath(__file__)), given)):
            if is_run(candidate):
                return candidate
        matches = [run for run in runs if given.lower() in run.lower()]
        if len(matches) == 1:
            return matches[0]
        if len(matches) > 1:
            report_runs(runs, "%r matches more than one run." % given)
        report_runs(runs, "%r is not a run folder." % given)
    return None


def report_runs(runs, problem):
    root = os.path.dirname(os.path.abspath(__file__))
    lines = [problem, ""]
    if runs:
        lines.append("Runs found, oldest first:")
        lines.extend("  %s" % os.path.relpath(run, root) for run in runs)
    else:
        lines.append("No runs found under %s." % root)
        lines.append("A run folder is one holding the .REGS.TSV the calculator wrote.")
    raise SystemExit("\n".join(lines))



# ---------------------------------------------------------------------------------------------------
# A workbook, written straight to the file format. openpyxl is not installed here and the sandbox will
# not let it be, so the few tags a plain data sheet needs are written out directly. Numbers go in as
# numbers and ms/call goes in as a formula, so the sheet still adds up if a figure is corrected.
# ---------------------------------------------------------------------------------------------------

def column_letter(index):
    letters = ""
    while index > 0:
        index, remainder = divmod(index - 1, 26)
        letters = chr(65 + remainder) + letters
    return letters


def escape(text):
    return (str(text).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
            .replace('"', "&quot;"))


def cell(reference, value, formula=False):
    if formula:
        return '<c r="%s"><f>%s</f></c>' % (reference, escape(value))
    if isinstance(value, (int, float)):
        return '<c r="%s"><v>%s</v></c>' % (reference, value)
    if value in (None, ""):
        return ""
    return '<c r="%s" t="inlineStr"><is><t>%s</t></is></c>' % (reference, escape(value))


def sheet_xml(rows, widths):
    out = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
           '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">']
    if widths:
        out.append("<cols>")
        for index, width in enumerate(widths, start=1):
            out.append('<col min="%d" max="%d" width="%d" customWidth="1"/>' % (index, index, width))
        out.append("</cols>")
    out.append('<sheetData>')
    for number, row in enumerate(rows, start=1):
        cells = []
        for index, value in enumerate(row, start=1):
            reference = "%s%d" % (column_letter(index), number)
            if isinstance(value, str) and value.startswith("="):
                cells.append(cell(reference, value[1:], formula=True))
            else:
                cells.append(cell(reference, value))
        out.append('<row r="%d">%s</row>' % (number, "".join(cells)))
    out.append("</sheetData></worksheet>")
    return "".join(out)


def write_workbook(path, sheets):
    """sheets: a list of (name, rows, column widths)."""
    import zipfile

    types = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
             '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">',
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>',
             '<Default Extension="xml" ContentType="application/xml"/>',
             '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-'
             'officedocument.spreadsheetml.sheet.main+xml"/>']
    books, links = [], []
    for index, (name, _rows, _widths) in enumerate(sheets, start=1):
        types.append('<Override PartName="/xl/worksheets/sheet%d.xml" ContentType="application/'
                     'vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>' % index)
        books.append('<sheet name="%s" sheetId="%d" r:id="rId%d"/>' % (escape(name), index, index))
        links.append('<Relationship Id="rId%d" Type="http://schemas.openxmlformats.org/officeDocument/'
                     '2006/relationships/worksheet" Target="worksheets/sheet%d.xml"/>' % (index, index))
    types.append("</Types>")

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as book:
        book.writestr("[Content_Types].xml", "".join(types))
        book.writestr("_rels/.rels",
                      '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                      '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                      '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                      'relationships/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        book.writestr("xl/workbook.xml",
                      '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                      '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
                      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
                      '<sheets>%s</sheets></workbook>' % "".join(books))
        book.writestr("xl/_rels/workbook.xml.rels",
                      '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                      '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                      '%s</Relationships>' % "".join(links))
        for index, (_name, rows, widths) in enumerate(sheets, start=1):
            book.writestr("xl/worksheets/sheet%d.xml" % index, sheet_xml(rows, widths))


PER_RUN = ("stack", "status", "span", "tenths", "calls", "ms/call")


def workbook_rows(runs, labels):
    """Every run side by side, one row per case. ms/call is a formula off that run's own case 01."""
    header = ["nr", "kind"]
    for label in labels:
        header.extend("%s %s" % (label, field) for field in PER_RUN)
    rows = [header]

    numbers = sorted({case["number"] for run in runs for case in run["cases"]})
    for position, number in enumerate(numbers):
        line = str(position + 2)                                   # the sheet row this case lands on
        name = ""
        for run in runs:
            for case in run["cases"]:
                if case["number"] == number:
                    name = case["name"]
        row = [number, name]
        for index, run in enumerate(runs):
            found = None
            for case in run["cases"]:
                if case["number"] == number:
                    found = case
            base = 3 + index * len(PER_RUN)
            tenths = column_letter(base + 3)
            calls = column_letter(base + 4)
            if found is None:
                row.extend(["", "", "", "", "", ""])
                continue
            row.extend([found["stack"], found["status"], found["span"], found["tenths"], found["calls"],
                        "=IF(%s%s=0,\"\",%s%s*100/%s%s-$%s$2*100/$%s$2)"
                        % (calls, line, tenths, line, calls, line, tenths, calls)])
        rows.append(row)
    return rows, [4, 12] + [9] * (len(PER_RUN) * len(runs))


def capture_rows(runs, labels):
    rows = [["run", "nr", "file", "bytes", "sha256"]]
    for label, run in zip(labels, runs):
        for index, capture in enumerate(run["captures"], start=1):
            rows.append([label, index, capture["name"], capture["bytes"], capture["sha256"]])
    return rows, [28, 4, 26, 9, 20]


def report(runs, labels, root, tolerance, stack_tolerance):
    """Everything, without being asked twice: the table of every run, the differences between each run
    and the one before it, into a text file, and the same figures into a workbook."""
    import io

    text = io.StringIO()
    held = sys.stdout
    sys.stdout = text
    try:
        for label, run in zip(labels, runs):
            print("=" * 96)
            print("RUN  %s" % label)
            print("=" * 96)
            show(run)
            print()
        # Only runs from the same machine folder are compared. A DM42 against a DM42n is not a
        # difference, it is two different calculators, and the two never share a figure.
        for index in range(1, len(runs)):
            if os.path.dirname(labels[index]) != os.path.dirname(labels[index - 1]):
                continue
            print("=" * 96)
            print("CHANGES  %s  against  %s" % (labels[index], labels[index - 1]))
            print("=" * 96)
            compare(runs[index - 1], runs[index], tolerance, stack_tolerance)
            print()
    finally:
        sys.stdout = held

    written = os.path.join(root, "report.txt")
    with open(written, "w", encoding="utf-8") as handle:
        handle.write(text.getvalue())

    book = os.path.join(root, "runs.xlsx")
    cases, case_widths = workbook_rows(runs, labels)
    captures_out, capture_widths = capture_rows(runs, labels)
    write_workbook(book, [("Cases", cases, case_widths), ("Captures", captures_out, capture_widths)])

    print("%d runs: %s" % (len(runs), ", ".join(labels)))
    print("wrote report.txt   every run, and what changed between each and the one before")
    print("wrote runs.xlsx    the same figures side by side, ms/call as a formula")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("action", nargs="?", default="report",
                        choices=("report", "show", "compare", "list"),
                        help="report writes report.txt and runs.xlsx for every run. Default report")
    parser.add_argument("run", nargs="?", help="a run folder. Left out, the newest is used")
    parser.add_argument("against", nargs="?", help="compare: the second run folder")
    parser.add_argument("--tolerance", type=float, default=DEFAULT_TIME_TOLERANCE,
                        help="fraction the loop time may move before it is reported")
    parser.add_argument("--stack-tolerance", type=int, default=DEFAULT_STACK_TOLERANCE,
                        help="bytes the stack figure may move before it is reported")
    args = parser.parse_args()

    runs = all_runs()
    root = os.path.dirname(os.path.abspath(__file__))

    if args.action == "report":
        if not runs:
            report_runs(runs, "Nothing to report.")
        return report([collect(run) for run in runs], [os.path.relpath(run, root) for run in runs],
                      root, args.tolerance, args.stack_tolerance)

    if args.action == "list":
        if not runs:
            report_runs(runs, "Nothing to list.")
        print("Runs found, oldest first:")
        for run in runs:
            print("  %s" % os.path.relpath(run, root))
        return 0

    if args.action == "show":
        where = resolve(args.run, runs) or (runs[-1] if runs else None)
        if not where:
            report_runs(runs, "Nothing to show.")
        print("run: %s" % os.path.relpath(where, root))
        return show(collect(where))

    first = resolve(args.run, runs)
    second = resolve(args.against, runs)
    if first is None and second is None:
        if len(runs) < 2:
            report_runs(runs, "compare needs two runs.")
        first, second = runs[-2], runs[-1]
    elif second is None:
        report_runs(runs, "compare needs a second run folder.")
    print("comparing: %s" % os.path.relpath(first, root))
    print("against:   %s" % os.path.relpath(second, root))
    return compare(collect(first), collect(second), args.tolerance, args.stack_tolerance)


if __name__ == "__main__":
    sys.exit(main())
