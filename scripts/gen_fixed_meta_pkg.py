#!/usr/bin/python3
"""
This script parses dependencies from debian/control file of consolinno-hems-latest package
and compiles consolinno-hems package based on current versions found in the package
repository
"""
from debian import deb822
import apt

cache = apt.cache.Cache()

for el in deb822.Deb822.iter_paragraphs(open("../debian/control")):
    if el.get('Package', None) == "consolinno-hems-latest":
        latest_entry = el
        break
new_deps = []
for dep in latest_entry['Depends'].split("\n"):
    dep = dep.strip().strip(",")
    if not dep.startswith("$"):
        new_deps.append(dep + " (= " + cache[dep].candidate.version + ")") 
    else:
        new_deps.append(dep)

new_entry = latest_entry
new_entry['Package'] = "consolinno-hems"
new_entry['Depends'] = " \n         ".join(new_deps)

print("\n\n")
print(new_entry)
