%{
for i in range(100):
  if i == 0:
    #define FOR_EACH0(...)
  else:
    #define FOR_EACH%{i}%(t, a, ...) t(a) FOR_EACH%{i-1}%(t, __VA_ARGS__)

for i in range(2, 100):
  if i == 2:
    #define FOR_EACH_2_0(...)
    #define P1_FOR_EACH_2_0(...)
    #define FOR_EACH_2_1(...)
    #define P1_FOR_EACH_2_1(...)
    #define FOR_EACH_2_2(t, a, b, ...) t(a, b)
    #define P1_FOR_EACH_2_2(t, a, b, c, ...) t(a, b, c) 
  else:
    #define FOR_EACH_2_%{i}%(t, a, b, ...) t(a, b) FOR_EACH_2_%{i-2}%(t, __VA_ARGS__)
    #define P1_FOR_EACH_2_%{i}%(t, a, b, c, ...) t(a, b, c) P1_FOR_EACH_2_%{i-2}%(t, a, __VA_ARGS__)

for i in range(100):
  if i == 0:
    #define P1_FOR_EACH0(...)
  else:
    #define P1_FOR_EACH%{i}%(t, a, b, ...) t(a, b) P1_FOR_EACH%{i-1}%(t, a, __VA_ARGS__)

#define NUM_ARGS_H1(x, %{','.join(['x'+str(100-i-1) for i in range(100)])}%, ...) x0 
#define NUM_ARGS(...) NUM_ARGS_H1(dummy, ##__VA_ARGS__, %{','.join([str(100-i-1) for i in range(100)])}%)
}%

%{
for prefix in ['', 'P1_']:
  for suffix in ['', '_2']:
    suffix2 = suffix
    if suffix == '_2':
      suffix2 += '_'
    preargs = ''
    if(prefix == 'P1_'):
      preargs = 'a,'
    #define %{prefix}%APPLY_ALL%{suffix}%_H3(t,%{preargs}% n, ...) %{prefix}%FOR_EACH%{suffix2}%##n(t,%{preargs}% __VA_ARGS__)
    #define %{prefix}%APPLY_ALL%{suffix}%_H2(t,%{preargs}% n, ...) %{prefix}%APPLY_ALL%{suffix}%_H3(t,%{preargs}% n, __VA_ARGS__)
    #define %{prefix}%APPLY_ALL%{suffix}%(t,%{preargs}%...) %{prefix}%APPLY_ALL%{suffix}%_H2(t,%{preargs}% NUM_ARGS(__VA_ARGS__), __VA_ARGS__)      
}%


