import matplotlib
import matplotlib.pyplot as plt
import numpy as np

from matplotlib.transforms import Affine2D

plt.style.use('bmh')

matplotlib.use("pgf")
matplotlib.rcParams.update({
    "pgf.texsystem": "pdflatex",
    'font.family': 'serif',
    'text.usetex': True,
    'pgf.rcfonts': False,
    'font.size' : 18
})


plt.gca().spines['top'].set_visible(False)
plt.gca().spines['right'].set_visible(False)

start = 3
end = 20
cycles_per_second = 4200328733

fig, ax = plt.subplots()
ax.grid(False)

t1 = Affine2D().translate(-0.2, 0.0) + ax.transData
t2 = Affine2D().translate(+0.2, 0.0) + ax.transData

apon_dic = {}

for i in range(start, end+1):
    with open('aponkex-data/{}.txt'.format(i), 'r') as f:
        apon_dic[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [apon_dic[i].mean()/(1e6) for i in x]
e = [apon_dic[i].std()/(1e6) for i in x]


#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="black", label="AponGKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="black", label="AponGKE", transform=t2)
#-----------------------------------------

choi_dic = {}

for i in range(start, end+1):
    with open('choigke-data/{}.txt'.format(i), 'r') as f:
        choi_dic[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [choi_dic[i].mean()/(1e6) for i in x]
e = [choi_dic[i].std()/(1e6) for i in x]

#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, label="ChoiGKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, label="ChoiGKE", transform=t1)

#---------------------------------------

dh_dic = {}

for i in range(start, end+1):
    with open('dh-data/{}.txt'.format(i), 'r') as f:
        dh_dic[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [dh_dic[i].mean()/(1e6) for i in x]
e = [dh_dic[i].std()/(1e6) for i in x]


#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="green", label="ECDH GKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="green", label="ECDH GKE")

#--------------------------------

plt.ylabel("Total CPU cycles (millions)")
plt.xlabel("Number of participants")
plt.legend()
plt.show()


def create_table(data_dic):

    clean_table = """\\begin{table}[]\n\\centering
    \\begin{tabular}{| l | l | l |} \\hline
    Participants & Mean CPU cycles & Standard deviation\\\\ \\hline \n"""

    dh_table = clean_table
    for i in range(start, end+1):
        dh_table += str(i) + " & {:.0f} & {:.0f}\\\\\n".format(data_dic[i].mean(), data_dic[i].std())

    dh_table += "\\hline\n \\end{tabular}\n\\end{table}"

    print(dh_table)


create_table(dh_dic)
create_table(apon_dic)
create_table(choi_dic)

fig.set_size_inches(w=12, h=6)
fig.savefig("gke-cyclesgraph.pdf", bbox_inches='tight')
