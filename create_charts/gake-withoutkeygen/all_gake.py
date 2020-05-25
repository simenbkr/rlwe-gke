import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.transforms import Affine2D

plt.style.use('bmh')
#plt.style.use("classic")
#plt.style.use("fast")

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

fix, ax = plt.subplots()

ax.grid(False)
t1 = Affine2D().translate(-0.20, 0.0) + ax.transData
t2 = Affine2D().translate(-0.40, 0.0) + ax.transData
t3 = Affine2D().translate(+0.20, 0.0) + ax.transData
t4 = Affine2D().translate(+0.40, 0.0) + ax.transData


start = 3
end = 20
cycles_per_second = 4200328733

apon_dic = {}

for i in range(start, end+1):
    with open('apon-data/{}.txt'.format(i), 'r') as f:
        apon_dic[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [apon_dic[i].mean()/(1e6) for i in x]
e = [apon_dic[i].std()/(1e6) for i in x]


#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="black", label="FALCON-AponGAKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="black", label="FALCON-AponGAKE", transform=t1)
#-----------------------------------------

choi_dic = {}

for i in range(start, end+1):
    with open('choi-data/{}.txt'.format(i), 'r') as f:
        choi_dic[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [choi_dic[i].mean()/(1e6) for i in x]
e = [choi_dic[i].std()/(1e6) for i in x]

#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, label="FALCON-ChoiGAKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, label="FALCON-ChoiGAKE", transform=t2)

#---------------------------------------

dh_dic = {}

for i in range(start, end+1):
    with open('dh-data/{}.txt'.format(i), 'r') as f:
        dh_dic[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [dh_dic[i].mean()/(1e6) for i in x]
e = [dh_dic[i].std()/(1e6) for i in x]


#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="green", label="ECDH GAKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, color="green", label="ECDH GAKE")
#--------------------------------

qapon = {}

for i in range(start, end+1):
    with open('qtesla-apon-data/{}.txt'.format(i), 'r') as f:
        qapon[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [qapon[i].mean()/(1e6) for i in x]
e = [qapon[i].std()/(1e6) for i in x]


#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2,  label="qTESLA-AponGAKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2,  label="qTESLA-AponGAKE", transform=t3)
#--------------------------------

qchoi = {}

for i in range(start, end+1):
    with open('qtesla-choi-data/{}.txt'.format(i), 'r') as f:
        qchoi[i] = np.array([int(x) for x in f.read().strip(',').split(',')])

x = [i for i in range(start,end+1)]
plt.xticks(x)
y = [qchoi[i].mean()/(1e6) for i in x]
e = [qchoi[i].std()/(1e6) for i in x]


#plt.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, label="qTESLA-ChoiGAKE")
ax.errorbar(x, y, e, linestyle='None',marker='^', capsize=2, label="qTESLA-ChoiGAKE", transform=t4)

#--------------------------------


plt.ylabel("Total CPU cycles (millions)")
plt.xlabel("Number of participants")
plt.legend()
plt.show()

def create_table(data_dic):

    clean_table = """\\begin{table}[h!]\n\\centering
    \\begin{tabular}{| l | r | r |} \\hline
    Participants & Mean CPU cycles & Standard deviation\\\\ \\hline \n"""

    table = clean_table
    for i in range(start, end+1):
        table += str(i) + " & {:.0f} & {:.0f}\\\\\n".format(data_dic[i].mean(), data_dic[i].std())

    table += "\\hline\n \\end{tabular}\n\\end{table}"

    print(table)



print("\\section{GAKE without keygen}")
print("\\subsection{ECDHGAKE}")
create_table(dh_dic)
print("\\newpage\n\n")

print("\\subsection{FALCON-\\apongake{}}")
create_table(apon_dic)
print("\\newpage\n\n")

print("\\subsection{FALCON-\\choigake{}}")
create_table(choi_dic)
print("\\newpage\n\n")

print("\\subsection{qTESLA-\\apongake{}}")
create_table(qapon)
print("\\newpage\n\n")

print("\\subsection{qTESLA-\\choigake{}}")
create_table(qchoi)
print("\\newpage\n\n")



fix.set_size_inches(w=12, h=6)
fix.savefig("gake-withoutkeygen.pdf", bbox_inches='tight')
