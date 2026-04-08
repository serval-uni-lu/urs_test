import pandas as pd
import numpy as np
from mpmath import mp
from statistics import median
from statistics import mean
import re

mp.pretty = True

mc = pd.read_csv("cnf.mc.csv", skipinitialspace = True, index_col = 'file')
clt = pd.read_csv("cnf.clt.csv", skipinitialspace = True, index_col = 'file')
# glmc = pd.read_csv("cnf.lmc.csv", skipinitialspace = True, index_col = 'file')
bounds = pd.read_csv("cnf.bounds.csv", skipinitialspace = True, index_col = 'file')
cls = pd.read_csv("cnf.cls.csv", skipinitialspace = True, index_col = 'file')
total = pd.read_csv("total.csv", skipinitialspace = True)

mc.dropna(inplace = True)
clt.dropna(inplace = True)
cls.dropna(inplace = True)
# glmc.dropna(inplace = True)
bounds.dropna(inplace = True)
total.dropna(inplace = True)

d = clt.join(mc, on = 'file')
# d = d.join(glmc, on = 'file')
d = d.join(bounds, on = 'file')
d.dropna(inplace = True)

def format_float(v):
    return f"{v:5.3f}"

def left_padder(width):
    return lambda x : ((width - len(x)) * ' ') + x

def right_padder(width):
    return lambda x : x + ((width - len(x)) * ' ')

def pad_left(l):
    return list(map(left_padder(max(map(len, l))), l))

def pad_right(l):
    return list(map(right_padder(max(map(len, l))), l))

def to_latex_table(table):
    for i in table:
        table[i] = pad_right(table[i])

    for l in zip(*table.values()):
        print(' & '.join(l), "\\\\")

def to_md_table(table):
    for i in table:
        table[i] = pad_right(table[i])

    i = 0
    for l in zip(*table.values()):
        if i == 1:
            lp = list(map(lambda x : '-' * len(x), l))
            print('|' + ' | '.join(lp) + '|')
        print('|' + ' | '.join(l) + '|')
        i += 1


def comp_general_table():
    tab = { 'sub': ['Dataset']
           , 'nbf': ['#F<sub>total</sub>']
           , 'v_lo': ['min(&#124;Var(F)&#124;)']
           , 'v_hi': ['max(&#124;Var(F)&#124;)']
           , 'c_lo': ['min(&#124;F&#124;)']
           , 'c_hi': ['max(&#124;F&#124;)']
           }

    for x in total.index:
        sub = total['folder'][x]
        nbf = total['nbf'][x]
        vsub = total['map'][x]

        vsub = vsub.replace('$\\hookrightarrow$', '↪')
        vsub = vsub.replace('$\\hookrightarrow \\hookrightarrow$', '↪↪')
        vsub = re.sub(r'\\cite\{.*?\}', '', vsub)

        lcls = cls[cls.index.str.contains(sub)]

        v_lo = int(min(lcls['#v']))
        v_hi = int(max(lcls['#v']))
        v_avg = int(mean(lcls['#v']))
        v_med = int(median(lcls['#v']))

        c_lo = int(min(lcls['#c']))
        c_hi = int(max(lcls['#c']))
        c_avg = int(mean(lcls['#c']))
        c_med = int(median(lcls['#c']))

        # print(f"{vsub} & {nbf} & {v_lo} & {v_hi} & {c_lo} & {c_hi} \\\\")
        tab['sub'].append(str(vsub))
        tab['nbf'].append(str(nbf))
        tab['v_lo'].append(str(v_lo))
        tab['v_hi'].append(str(v_hi))
        tab['c_lo'].append(str(c_lo))
        tab['c_hi'].append(str(c_hi))

    return tab

def comp_comp_table():
    tab = dict()
    tab['vsub'] = ['Dataset']
    tab['nbf'] = ['#F<sub>total</sub>']
    tab['d4'] = ['#D4 and not DivKC']
    tab['nd4'] = ['#not D4']
    tab['divkc'] = ['#DivKC and not D4']

    for x in total.index:
        sub = total['folder'][x]
        nbf = total['nbf'][x]
        vsub = total['map'][x]

        vsub = vsub.replace('$\\hookrightarrow$', '↪')
        vsub = vsub.replace('$\\hookrightarrow \\hookrightarrow$', '↪↪')
        vsub = re.sub(r'\\cite\{.*?\}', '', vsub)

        ldf = d[d.index.str.contains(sub)]
        lmc = mc[mc.index.str.contains(sub)]
        lgb = clt[clt.index.str.contains(sub)]

        nmc = len(lmc)

        slgb = set(lgb.index)
        slmc = set(lmc.index)

        k = len(slgb - slmc)

        nd4 = len(slmc)
        nt = len(slgb)
        nboth = len(slmc.intersection(slgb))

        onlyd4 = len(slmc - slgb)
        no = len(slgb | slmc)

        # if k > 0:
        #     k = "\\textbf{" + str(k) + "}"

        tab['vsub'].append(str(vsub))
        tab['nbf'].append(str(nbf))
        tab['d4'].append(str(onlyd4))
        tab['nd4'].append(str(nbf - len(lmc)))
        tab['divkc'].append(str(k))

    return tab

def comp_clt_table():
    tab = dict()
    tab['vsub'] = ['Dataset']
    tab['nbf'] = ['#F']
    tab['nlow'] = ['Y<sub>l</sub> &le; &#124;R<sub>F</sub>&#124;']
    tab['nhigh'] = ['Y<sub>h</sub> &ge; &#124;R<sub>F</sub>&#124;']
    tab['nboth'] = ['Coverage']
    tab['ntboth'] = ['&#124;R<sub>G<sub>P</sub></sub>&#124; &le; &#124;R<sub>F</sub>&#124; &le; &#124;R<sub>G<sub>U</sub></sub>&#124; ']

    for x in total.index:
        sub = total['folder'][x]
        nbf = total['nbf'][x]
        vsub = total['map'][x]

        vsub = vsub.replace('$\\hookrightarrow$', '↪')
        vsub = vsub.replace('$\\hookrightarrow \\hookrightarrow$', '↪↪')
        vsub = re.sub(r'\\cite\{.*?\}', '', vsub)

        ldf = d[d.index.str.contains(sub)]
        lmc = mc[mc.index.str.contains(sub)]
        lgb = clt[clt.index.str.contains(sub)]

        nmc = len(lmc)

        slgb = set(lgb.index)
        slmc = set(lmc.index)

        k = len(slgb - slmc)

        nlow = 0
        nhigh = 0
        nboth = 0
        nb = 0

        ntlow = 0
        nthigh = 0
        ntboth = 0

        for y in ldf.index:
            yl = mp.mpf(ldf.Yl[y])
            yh = mp.mpf(ldf.Yh[y])
            lo = mp.mpf(ldf.low[y])
            hi = mp.mpf(ldf.high[y])
            tm = mp.mpf(ldf.mc[y])

            # ntlow += (tm >= lo)
            # nthigh += (tm <= hi)
            ntboth += (tm >= lo) and (tm <= hi)

            nlow += yl <= tm
            nhigh += yh >= tm
            nboth += (yh >= tm) and (yl <= tm)
            nb += 1


        if nb > 0:
            nlow /= nb
            nhigh /= nb
            nboth /= nb

            # ntlow /= nb
            # nthigh /= nb
            ntboth /= nb

            if k > 0:
                k = "\\textbf{" + str(k) + "}"
            # print(f"{vsub} & {nbf} & {len(lmc)} & {k} & {nlow:5.3f} & {nhigh:5.3f} & {nboth:5.3f} & {ntlow:5.3f} \\\\")
            # print(f"{vsub} & {nb} & {nlow:5.3f} & {nhigh:5.3f} & {nboth:5.3f} & {ntboth:5.3f} \\\\")
            tab['vsub'].append(str(vsub))
            tab['nbf'].append(str(nb))
            tab['nlow'].append(format_float(nlow))
            tab['nhigh'].append(format_float(nhigh))
            tab['nboth'].append(format_float(nboth))
            tab['ntboth'].append(format_float(ntboth))
        else:
            tab['vsub'].append(str(vsub))
            tab['nbf'].append(str(nb))
            tab['nlow'].append("")
            tab['nhigh'].append("")
            tab['nboth'].append("")
            tab['ntboth'].append("")

    return tab

def comp_clt2_table():
    tab = dict()
    tab['vsub'] = ['Dataset']
    tab['nbf'] = ['#F']
    tab['nlow'] = ['Y<sub>l</sub> &ge; &#124;R<sub>G<sub>P</sub></sub>&#124;']
    tab['nhigh'] = ['Y<sub>h</sub> &le; &#124;R<sub>G<sub>U</sub></sub>&#124;']
    tab['nboth'] = ['Both']
    tab['medrc'] = ['median(r<sub>c</sub>)']
    tab['maxrc'] = ['max(r<sub>c</sub>)']

    for x in total.index:
        sub = total['folder'][x]
        nbf = total['nbf'][x]
        vsub = total['map'][x]

        vsub = vsub.replace('$\\hookrightarrow$', '↪')
        vsub = vsub.replace('$\\hookrightarrow \\hookrightarrow$', '↪↪')
        vsub = re.sub(r'\\cite\{.*?\}', '', vsub)

        ldf = d[d.index.str.contains(sub)]
        lmc = mc[mc.index.str.contains(sub)]
        lgb = clt[clt.index.str.contains(sub)]

        nlow = 0
        nhigh = 0
        nboth = 0
        nbothc = 0
        # nboth = 0
        # nlvl = 0
        # ntlow = 0
        nb = 0

        resl = []

        for y in ldf.index:
            yl = mp.mpf(ldf.Yl[y])
            yh = mp.mpf(ldf.Yh[y])
            lo = mp.mpf(ldf.low[y])
            hi = mp.mpf(ldf.high[y])
            tm = mp.mpf(ldf.mc[y])

            nlow += (yl >= lo) and (yl <= tm)
            nhigh += (yh <= hi) and (yh >= tm)
            nboth += (yl >= lo) and (yh <= hi)
            nbothc += (yl >= lo) and (yh <= hi) and (yl <= tm) and (yh >= tm)

            nb += 1
            if yh < yl:
                print(f"    error y: {y} : {yh} > {yl}")
            if hi < lo:
                print(f"    error h: {y} : {hi} > {lo}")

            # if (yl >= lo) and (yh <= hi) and (yl <= tm) and (yh >= tm):
            if (yl <= tm) and (yh >= tm):
                resl.append((min(yh, hi) - max(yl, lo)) / (hi - lo))


        if nb > 0:
            nlow /= nb
            nhigh /= nb
            # ntlow /= nb
            # nlvl /= nb
            # nboth /= nb
            nbothc /= nb

            # print(f"{vsub} & {nb} & {nlow:5.3f} & {nhigh:5.3f} & {nboth:5.3f} & {nbothc:5.3f} \\\\")
            # print(f"{vsub} & {nb} & {nlow:5.3f} & {nhigh:5.3f} & {nbothc:5.3f} & {mp.nstr(median(resl), 3)} & {mp.nstr(max(resl), 3)} \\\\")
            tab['vsub'].append(str(vsub))
            tab['nbf'].append(str(nb))
            tab['nlow'].append(format_float(nlow))
            tab['nhigh'].append(format_float(nhigh))
            tab['nboth'].append(format_float(nbothc))
            tab['medrc'].append(mp.nstr(median(resl), 3))
            tab['maxrc'].append(mp.nstr(max(resl), 3))

            # print(f"{vsub} & {nbf} & {len(lmc)} & {k} & {nlow:5.3f} & {nhigh:5.3f} & {nboth:5.3f} & {ntlow:5.3f} \\\\")
            # print(f"{vsub} & {nbf} & {len(lmc)} & {k} & {nlow:5.3f} & {nhigh:5.3f} & {nboth:5.3f} \\\\")
        else:
            # print(f"{vsub} & {nbf} & {len(lmc)} & {k} & & & \\\\")
            tab['vsub'].append(str(vsub))
            tab['nbf'].append(str(nb))
            tab['nlow'].append("")
            tab['nhigh'].append("")
            tab['nboth'].append("")
            tab['medrc'].append("")
            tab['maxrc'].append("")

    return tab

print("# Datasets")
print("## Table 1: Dataset Summary")
to_md_table(comp_general_table())
print("# Knowledge Compilation")
print("## Table 2: Experimental results regarding the scalability of DivKC")
to_md_table(comp_comp_table())
print("# Approximate Model Counting")
print("## Table 3: Experimental results for Algorithm 3")
to_md_table(comp_clt_table())
print("## Table 4: Experimental results comparing the bounds obtained with Algorithm 3 and with Lemma 1")
to_md_table(comp_clt2_table())
