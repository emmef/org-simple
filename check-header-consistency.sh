#!/bin/bash

pushd `dirname "$0"`

error_exit() {
  popd
  exit $1
}

checkSanity() {
  local fullPath
  local pathName
  local nameSpace
  local fileName
  local tempDirectory
  local correctNameInHeader
  local fileLineInHeader

  fullPath="$1"
  pathName=$(dirname "$fullPath")
  fileName=$(basename "$fullPath")

# Strip include from filename
  pathName=${pathName/.\/include\//}
  nameSpace=${pathName//\//::}
  nameSpace=${nameSpace//-/::}

  tempDirectory="/tmp/replace/include/${pathName}"

#  echo "File $fileName Path $pathName NameSpace $nameSpace"
  correctNameInHeader="${pathName}/${fileName}"
  fileLineInHeader=$(grep -E -A1 "^/\\*s*\$" "$fullPath" | grep -Ei "^ \* [-_A-Z0-9/]+\.h.*" "$fullPath")

#  echo "File line: '$fileLineInHeader'"

  if ! echo "$fileLineInHeader" | grep -E "^ \\* $correctNameInHeader\$" >/dev/null
  then
    echo -e "File ${fullPath} should use following file-name:\n\t${correctNameInHeader}" >&2
  fi

  if grep -E "^namespace\s+[a-z0-9A-Z:]+\s+{" "${fullPath}" >/dev/null
  then
    if ! grep -E "^namespace\s+${nameSpace}\s+\{" "${fullPath}" >/dev/null
    then
      echo -e "File ${fullPath} should use following namespace:\nnamespace ${nameSpace} {\n} // namespace ${nameSpace}" >&2
    fi
  fi
#
#  if [ ! -d "${tempDirectory}" ]
#  then
#    if ! mkdir -p "${tempDirectory}"
#    then
#      echo "Cannot create temporary directory: ${tempDirectory}" >&2
#      exit 1
#    fi
#  fi
#  if ! cp "$fullPath" "${tempDirectory}/$fileName"
#  then
#    echo "Cannot make copy ${tempDirectory}/${fileName}" >&2
#    exit 1
#  fi



}


for name in `find .| grep -Ei './include/.*\.h' | sort` ; do  checkSanity "$name" ; done

popd
