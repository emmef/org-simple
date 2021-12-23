#!/bin/bash

checkSanity() {
  local fullPath
  local pathName
  local nameSpace
  local fileName
  local tempDirectory
  local fileNameInheader
  
  fullPath="$1"
  pathName=$(dirname "$fullPath")
  fileName=$(basename "$fullPath")
  
# Strip include from filename
  pathName=${pathName/.\/include\//}
  nameSpace=${pathName//\//::}
  nameSpace=${nameSpace//-/::}
  
  tempDirectory="/tmp/replace/include/${pathName}"
  
#  echo "File $fileName Path $pathName NameSpace $nameSpace"
  fileNameInheader="${pathName}/${fileName}"
  if ! grep -E " * $fileNameInheader" "$fullPath" >/dev/null
  then
    echo -e "File ${fullPath} should use following file-name:\n\t${fileNameInheader}" >&2
  fi
  
  if grep -E "^namespace\s+[a-z0-9A-Z:]+\s+{" "${fullPath}" >/dev/null
  then 
    if ! grep -E "^namespace\s+${nameSpace}\s+\{" "${fullPath}" >/dev/null
    then 
      echo -e "File ${fullPath} should use following namespace:\nnamespace ${nameSpace} {\n} // namespace ${nameSpace}" >&2
    fi
  fi
  
}


 for name in `find .| grep -Ei './include/.*\.h' | sort` ; do  checkSanity "$name" ; done
 
