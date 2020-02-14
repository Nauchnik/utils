import glob
import os
import matplotlib.pyplot as plt

plotmarkers = ["H","^","x","+","D","*","o","d","v",">"]
def load_logfile(fn, filter=""):    
    res = list()    
    with open(fn,"r") as infile:
        for line in infile:                          
              if line[0]!="-" and line[0]!='v':
                line = line.strip("\r\n ")              
                p = line.split(" ")              
                res.append(p)                            
    return res

def load_logs(pathname):
    files = glob.glob(pathname+"*")
    res = list()
    #print (str(files))
    for u in files:
        res.append(load_logfile(u))
    return res
   
def list_dirs(pathname):
    pathlist = glob.glob(pathname+"*/")
   # print(pathlist)
    pathlist = [u.replace("\\","/") for u in pathlist]
    return pathlist
    
def plot_single(title, input, column, x_min, xlabel, plot_fn):
    
    max_x = 0
    max_y = 0
    min_x = x_min
    min_y = 1e300
    
    plot_data = dict()
    
    for vname in input:
        plot_data[vname] = list()
        for lf in input[vname]:
            x_data = list()
            y_data = list()
            for u in lf:
                if len(u) <= column:
                    continue
                x = float(u[column])
                if x < min_x:
                    continue
                x_data.append(x)
                y_data.append(float(u[1]))
                if max_y<float(u[1]):
                    max_y = float(u[1])
                if min_y>float(u[1]):
                    min_y = float(u[1])
                if max_x < x:
                    max_x = x
            plot_data[vname].append([x_data,y_data])
    if len(plot_data) == 0:
        return
    max_x = max_x+50
    
    min_y = min_y/10
    max_y = max_y*10
    palette = plt.get_cmap("Set1")

    k = 0
    for vname in plot_data:        
        in_label = vname
        for u in range(len(plot_data[vname])):        
            plt.plot(plot_data[vname][u][0],plot_data[vname][u][1],linewidth = 0.7,marker = plotmarkers[u],fillstyle='none', color=palette(k),label=in_label+"_"+str(u))
        k = k + 1
    plt.xlim(min_x,max_x)
    plt.grid(True)
    #plt.ylim(min_y,max_y)
    plt.xlabel(xlabel)
    plt.yscale('log')
    plt.ylabel("Time")
    plt.legend(loc='best')
    #if title !="":
    #    plt.title(title)
    #plt.show()
    plt.savefig(plot_fn)
    plt.clf()    


def plot_figures(pathname,column,problem_x_min : dict, aux_label = ""):            
    pl = list_dirs(pathname)    
    for problem_name in pl:
        results = dict()
        cleared_problem_name = ""
        cleared_problem_name += problem_name
        cleared_problem_name = cleared_problem_name.replace("/",'')
        cleared_problem_name = cleared_problem_name.replace(".",'')
        #print(cleared_problem_name)
        if cleared_problem_name not in problem_x_min:
            print(cleared_problem_name + " not in problem_x_min")
            exit()
        x_min = problem_x_min[cleared_problem_name]
        print(problem_name + ", x_min : " + str(x_min))
        fn = (problem_name.strip(' /')).split('/')[-1]
        plotname = pathname + fn + aux_label+".pdf"
        #print(fn)        
        pl2 = list_dirs(problem_name)
        print(pl2)
        for v in pl2:
            vname = (v.strip(' /')).split('/')[-1]
            print(vname)
           # print(vname)
            results[vname]=load_logs(v)
            #(title, in_label ,input, column, xlabel):    
        legend =""
        if column == 4:
            legend = "Computations of function"
        if column == 3:
            legend = "Time (seconds)"
        plot_single(fn,results,column, x_min, legend, plotname)

pathname = "./"

#pl = plot_figures(pathname,4,0,"_computations")
problem_x_min = dict()
problem_x_min['mickey'] = 0
problem_x_min['grain'] = 60
problem_x_min['trivium'] = 180
problem_x_min['wolfram'] = 0

pl = plot_figures(pathname,3,problem_x_min,"_general")
    