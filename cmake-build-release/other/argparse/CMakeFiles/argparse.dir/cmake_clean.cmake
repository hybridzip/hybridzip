file(REMOVE_RECURSE
  "libargparse.a"
  "libargparse.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/argparse.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
