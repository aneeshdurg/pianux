#!/usr/bin/python3
import re
from sys import argv
from os import system

class Struct():
    def __init__(self, source):
        self.source = source;
        source = clean(source);
        i = source.find('struct')
        i += len('struct')
        self.name = source[i:].split('{')[0] 
        inside = '{'.join(source.split('{')[1:])
        inside = '}'.join(inside.split('}')[:-1])
        inside = self.remove_curls(inside)
        vals = inside.split(';') 
        vals = list(map(lambda x: x.split(' ')[-1].split('*')[-1], vals))
        vals = list(filter(lambda x: len(x.strip()) != 0, vals))
        self.vals = vals 

    def remove_curls(self, s):
        cleaned = []
        skip = 0 
        for c in s:
            if c == '{':
                skip += 1
            elif c == '}':
                skip -= 1
                cleaned.append(' ')
            elif not skip:
                cleaned.append(c)
        return ''.join(cleaned)

class pymacro():
    def __init__(self, name):
        self.name = name
        self.args = []
        self.lines = []
    def toDefStr(self):
        self.args = list(map(lambda x:x.strip().split(' '), self.args))
        header = "def %s(%s):"%(self.name, ','.join([a[1] for a in ([('','o')]+self.args)]))
        argCreate = ""
        for a in self.args:
            argCreate += "\n  "
            argCreate += "if not check_defs(%s)(%s):\n    raise(Exception('Bad argument to %s'))"%(a[0], a[1], self.name)
            argCreate += "\n  "
            argCreate += "%s = %s(%s)"%(a[1], a[0][0].upper()+a[0][1:], a[1])
        self.lines = list(map(lambda s: escapes("output(o, \"\"\"%s\"\"\")"%s), self.lines))
        body = "\n  "
        body += body.join(self.lines)
        #self.lines = list(map(lambda x:x.strip().split(' '), self.args))
        function = header+argCreate+body
        return function

def fixpt(f, x):
    if x == f(x):
        return x
    return fixpt(f, f(x))

def no_doublespace(s):
    return fixpt(lambda x: x.replace('  ',' '), s)

def no_random_space(s):
    for c in ";{()}":
        s = s.replace(' '+c,c)
        s = s.replace(c+' ',c)
    return s

def clean(s):
    clean = s.strip().replace('\t','').replace('\n','')
    clean = no_doublespace(clean)
    clean = no_random_space(clean)
    return clean

def check_func(s):
    return bool(re.match(".* .*\(.*\)\{.*\}", clean(s)))

def check_struct(s):
    try:
        return clean(s).split('{')[0].split(' ')[-2] == 'struct'
    except:
        return False

def check_stmt(s):
    return True

function = 0
struct = 1
stmt = 2
mapping = {
    function:check_func,
    struct:  check_struct,
    stmt:    check_stmt
}


def check_defs(t):
    return mapping[t]

def assert_cond(f, x):
    return f(x)

def to_arg_list(l):
    return ','.join(l)

def output(o, string):
    o.write(string)

def escapes(string):
   string = '\"\"\"+str('.join(string.split("%{")) 
   string = ')+\"\"\"'.join(string.split("}%")) 
   return string

fn_defs = dict()

def process_buff(buff, o):
    for i in range(len(buff)):
        l = buff[i]
        r = re.match(r' *#define ', l)
        if r:
            if(l.strip().split(' ')[1] == 'pymacro'):
                defline = ' '.join(l.strip().split(' ')[2:])
                newMacro = pymacro(defline.split('(')[0].strip())
                arglist = ')'.join(
                        '('.join(defline.split('(')[1:]).split(')')[:-1]) 
                arglist = arglist.split(",")
                arglist = list(map(lambda s: s.strip(), arglist))
                newMacro.args = arglist
                start = i
                i += 1
                while i < len(buff) and buff[i] != "#enddef\n":
                    l = buff[i].strip()
                    newMacro.lines.append(l)
                    buff[i] = ""
                    i += 1

                if i >= len(buff) or buff[i] != "#enddef\n":
                    raise(Exception("Bad definition..."))

                s = newMacro.toDefStr()
                print(s) 
                exec(s) 
                print("fn_defs[\"%s\"] = %s"%(newMacro.name , newMacro.name))
                exec("fn_defs[\"%s\"] = %s"%(newMacro.name , newMacro.name))
                continue
            buff[i] =\
                    r.string[:r.span()[1]-len("#define")-1] + \
                    "output(o, \"\"\"#define "+\
                    r.string[r.span()[1]:] + \
                    "\"\"\")\n"
            buff[i] =\
                    escapes(buff[i])

        else:
            r = re.match(r' *#run pymacro ', l)
            if r:
                fnname = l.strip().split(' ')[2].split('(')[0]
                if fnname not in fn_defs:
                    print("FN %s not found!"%fnname)
                    exit()
                lcount = 1
                rcount = 0
                l = '('.join(l.strip().split('(')[1:])
                start = i
                while i < len(buff) and rcount < lcount:
                    for c in l:
                        if c == ')':
                            rcount += 1
                        elif c == '(':
                            lcount += 1
                        if rcount == lcount:
                            break
                    if rcount != lcount:
                        i+=1
                        if i < len(buff):
                            l = buff[i]
                fn_call = ''.join(buff[start:i+1])
                fn_args = '('.join(fn_call.split('(')[1:])
                fn_args = ')'.join(fn_args.split(')')[:-1])
                j = start
                while j <= i:
                    if j < len(buff):
                        buff[j] = ""
                    j+=1
                fn_defs[fnname](o, fn_args)

    cmd = ''.join(buff)
    exec(cmd)


def main():
    if(len(argv) == 1):
        print("No input files!")
        exit()
    for filename in argv[1:]:
      if(filename[-2:] not in ['.c','.h']):
          print("Only .c and .h files supported!")
          exit()
      if(filename[-6:-2] != '.pyp'):
          print("Only .pyp.(c/h) files supported!")
          exit()

      f = open(filename, 'r')
      o = open(filename[:-6]+filename[-2:], 'w')
      lines = f.readlines()
      buff = []
      record = False
      for i in range(len(lines)):
          l = lines[i]
          if l == "%{\n":
              record = True
          elif l == "}%\n":
              process_buff(buff, o)
              record = False 
              buff = []
          else:
              if record:
                  buff.append(l)
              else:
                  o.write(l)
      f.close()
      o.close()
      outputname = filename[:-6]+filename[-2:]
      cleanname = outputname+".clean"
      system("clang-format %s > %s"%(outputname, cleanname))
      system("mv %s %s"%(cleanname, outputname))

if __name__ == "__main__":
    main()
