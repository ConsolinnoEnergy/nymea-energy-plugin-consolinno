 #!/bin/bash
 apt -y install consolinno-hems && apt install $(apt depends consolinno-hems | awk '{print $4}')
