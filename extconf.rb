# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

#COMPILE_C = "#{COMPILE_C} -framework IOKit -framework Carbon"

$LIBRUBYARG_SHARED = "#{$LIBRUBYARG_SHARED} -framework IOKit -framework Carbon"

# Give it a name
extension_name = 'irrb'

# The destination
dir_config(extension_name)

# Do the work
create_makefile(extension_name)
