
class DIMACS:
    def __init__(self):
        self.nb_vars = 0
        self.cls = []

    def to_file(self, path):
        with open(path, "w") as out:
            out.write(f"p cnf {self.nb_vars} {len(self.cls)}\n")

            for c in self.cls:
                for i in c:
                    out.write(f"{i} ")
                out.write("0\n")


def from_file(path):
    res = DIMACS()

    with open(path, 'r') as file:
        for line in file:
            line = line.strip()

            if line.startswith("p cnf "):
                res.nb_vars = int(line.split(' ')[2])
            elif not line.startswith("c"):
                c = list(set([int(x) for x in line.split(' ') if x != '0']))
                res.cls.append(c)

    return res
