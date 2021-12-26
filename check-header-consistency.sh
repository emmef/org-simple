#!/bin/bash


synopsis() {
  echo -e "USAGE:\n\t$0 [--silent] [--fix-errors]"
}

errorExit() {
  if [ -n "$1" ]
  then
    echo "$*" >&2
  fi
  exit 1
}

errorExitSynopsis() {
  synopsis
  if [ -n "$1" ]
  then
    echo "$*" >&2
  fi
  exit 1
}

moveToScriptDirectory() {
  local myDir
  myDir=$(dirname $0)
  if  ! cd "$myDir"
  then
    errorExit "Could not change directory to \"$myDir\""
  fi
}

# Global variables that alter behaviour of this script
fixErrors=
silentFix=

parseArguments() {
  local arg
  while [ -n "$1" ]
  do
    arg="$1"
    shift
    case "$arg" in
      --silent|-S)
        silentFix="true"
        ;;
      --fix-errors|-F)
        fixErrors="true"
        ;;
      *)
       errorExitSynopsis "Unrecognised argument: " "$arg"
    esac
  done
}

parseArguments $*
moveToScriptDirectory

createTemporaryFile() {
  local tempDirectory
  tempDirectory="$1/$headerDirectory"
  local copyFullPath
  copyFullPath="$tempDirectory/$headerBaseName"

  if [ ! -d "${tempDirectory}" ]
  then
    if ! mkdir -p "${tempDirectory}"
    then
      echo "Cannot create temporary directory: ${tempDirectory}" >&2
      exit 1
    fi
  fi
  if ! cp "$headerFullPath" "$copyFullPath"
  then
    echo "Cannot make copy ${tempDirectory}/${fileName}" >&2
    exit 1
  fi
  echo "$copyFullPath"
}

replaceFileNameLine() {
  local processedFile
  processedFile="$(dirname "$copiedFile")/.$(basename "$copiedFile")"
  echo "Using $processedFile"

  if [ -f "$processedFile" ]
  then
    rm "$processedFile"
  fi

  local line
  while IFS= read -r line
  do
    if [ "$fileLineInHeader" == "$line" ]
    then
      echo "Replaced '$fileLineInHeader' -> ' * $headerStatedFileName'"
      echo " * $headerStatedFileName" >> "$processedFile"
    else
      echo "$line"  >> "$processedFile"
    fi
  done

  cp "$processedFile" "$headerFullPath"
}

fixFileNameInHeader() {
  local copiedFile

  copiedFile=$(createTemporaryFile "/tmp/fix-filename-in-header")
  replaceFileNameLine < "$headerFullPath"
}

checkFileNameInHeader() {
  local fileLineInHeader

  IFS= read -r fileLineInHeader < <(grep -E -A1 "^/\\*s*\$" "$headerFullPath" | grep -Ei "^\s+\*\s+[-_A-Z0-9/]+\.h\s*" "$headerFullPath")

  if ! echo "$fileLineInHeader" | grep -E "^\s+\\*\s+$headerStatedFileName\s*\$" >/dev/null
  then
    if [ -z "$silentFix" ]
    then
      echo -e "PROBLEM: \"${headerFullPath}\" should use following file-name:\n\t${headerStatedFileName}" >&2
    fi
    if [ -n "$fixErrors" ]
    then
      fixFileNameInHeader
    fi
  fi
}

readNamespaceStartAndEnd() {
  local line
  local match
  local endifMatch
  local emptyLineMatch
  local guard
  local endifLine

  emptyLineMatch='^\s*(//|)\s*$'
  match='^\s*#ifndef\s+([_A-Z0-9]+)\s*$'

  while IFS= read -r line
  do
    if [[ $line =~ $match ]]
    then
      if [ -z "$guard" ]
      then
        guard="${BASH_REMATCH[1]}"
        match='^\s*namespace\s+([a-z0-9A-Z:]+)\s+\{\s*$'
        endifMatch="^#endif\\s+//\\s+$guard\\s*\$"
#        echo -e "FOUND $guard IN $line\n\tNow matching: $match"
      elif [ -z "$lineNamespaceStart" ]
      then
        lineNamespaceStart="$line"
        namespaceInFile="${BASH_REMATCH[1]}"
        match='}\s+//\s+namespace\s+[a-z0-9A-Z:]+\s*$'
#        echo -e "FOUND $namespaceInFile IN $line\n\tNow matching: $match"
      else
#        echo "FOUND namespace-end IN $line"
        lineNamespaceEnd="$line"
      fi
    elif [ -n "$lineNamespaceEnd" ]
    then
      if [[ $line =~ $endifMatch ]]
      then
#        echo "FOUND $endifMatch IN $line"
        endifLine="$line"
        return 0
      fi
    fi
  done
  if [ -n "$lineNamespaceEnd" ] && [ -n "$endifLine" ]
  then
    return 0
  fi

  return 1
}

replaceNameSpaceLines() {
  local processedFile
  processedFile="$(dirname "$copiedFile")/.$(basename "$copiedFile")"

  if [ -f "$processedFile" ]
  then
    rm "$processedFile"
  fi

  local line
  while IFS= read -r line
  do
    if [ "$lineNamespaceStart" == "$line" ]
    then
#      echo "Replaced '$line' -> 'namespace $headerNamespace {'"
      echo "namespace $headerNamespace {" >> "$processedFile"
    elif [ "$lineNamespaceEnd" == "$line" ]
    then
#      echo "Replaced '$line' -> '| // namespace $headerNamespace"
      echo "} // namespace $headerNamespace" >> "$processedFile"
    else
      echo "$line" >> "$processedFile"
    fi
  done

  cp "$processedFile" "$headerFullPath"
}

fixNamespaceInHeader() {
  local copiedFile

  copiedFile=$(createTemporaryFile "/tmp/fix-namespace-in-header")
  echo "Fix $headerFullPath"
  replaceNameSpaceLines < "$headerFullPath"
}

checkNamespaceInHeader() {
  local lineNamespaceStart
  local lineNamespaceEnd
  local namespaceInFile

  lineNamespaceEnd=
  if readNamespaceStartAndEnd < "$headerFullPath"
  then
    if [ "$namespaceInFile" != "$headerNamespace" ]
    then
      if [ -z "$silentFix" ]
      then
        echo -e "PROBLEM: Should use $headerNamespace instead of '$lineNamespaceStart' /* declarations */ '$lineNamespaceEnd'"
      fi
      if [ -n "$fixErrors" ]
      then
        fixNamespaceInHeader
      fi
    fi
  fi


#  if grep -E "^namespace\s+[a-z0-9A-Z:]+\s+{" "${headerFullPath}" >/dev/null
#  then
#    if ! grep -E "^namespace\s+${nameSpace}\s+\{" "${headerFullPath}" >/dev/null
#    then
#      echo -e "File ${headerFullPath} should use following namespace:\nnamespace ${nameSpace} {\n} // namespace ${nameSpace}" >&2
#    fi
#  fi

}

checkSanity() {
  local headerFullPath
  headerFullPath="$1"
  local headerBaseName
  headerBaseName=$(basename "$headerFullPath")
  local headerDirectory
  headerDirectory=$(dirname "$headerFullPath")
  # Strip "./include/"
  headerDirectory=${headerDirectory/.\/include\//}
  local headerNamespace
  headerNamespace=${headerDirectory//\//::}
  headerNamespace=${headerNamespace//-/::}
  local headerStatedFileName
  headerStatedFileName="$headerDirectory/$headerBaseName"
  local headerGuard
  headerGuard=${headerStatedFileName//\//_}
  headerGuard=${headerGuard//\./_}
  headerGuard=${headerGuard//\-/_}
  headerGuard=$(echo "$headerGuard" | tr '[:lower:]' '[:upper:]')

# Demonstration on how to read from a function output:
#  read -r headerFullPath headerBaseName headerDirectory headerNamespace headerStatedFileName headerGuard < <(generateHeaderInfo "$1")

  checkNamespaceInHeader
  checkFileNameInHeader
}




#for name in `find .| grep -Ei './include/.*\.h' | sort` ; do  generateHeaderInfo "$name" ; done
 for name in `find .| grep -Ei './include/.*\.h' | sort` ; do  checkSanity "$name" ; done

