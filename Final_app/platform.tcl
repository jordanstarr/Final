# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\Jordan\OneDrive\College\Engineering\SoC\Final\Final_app\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\Jordan\OneDrive\College\Engineering\SoC\Final\Final_app\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {Final_app}\
-hw {C:\Users\Jordan\OneDrive\College\Engineering\System on Chip\FinalProject\yippeKayay.xsa}\
-proc {microblaze_I} -os {standalone} -out {C:/Users/Jordan/OneDrive/College/Engineering/SoC/Final}

platform write
platform generate -domains 
platform active {Final_app}
platform generate
platform generate
