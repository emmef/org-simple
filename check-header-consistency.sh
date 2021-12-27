#!/bin/bash

synopsis() {
  echo -e "USAGE:\n\t$0 [--silent|-S] [--fix-errors|-F] [--verbose|-V] [--dry-run|-D]"
}

errorExit() {
  if [ -n "$1" ]; then
    echo "$*" >&2
  fi
  exit 1
}

errorExitSynopsis() {
  synopsis
  if [ -n "$1" ]; then
    echo "$*" >&2
  fi
  exit 1
}

moveToScriptDirectory() {
  local myDir
  myDir=$(dirname $0)
  if ! cd "$myDir"; then
    errorExit "Could not change directory to \"$myDir\""
  fi
}

# Global variables that alter behaviour of this script
fixErrors=
silentFix=
debugLevel=
dryRun=
backupRoot="/tmp/header-fixes"

parseArguments() {
  local arg
  while [ -n "$1" ]; do
    arg="$1"
    shift
    case "$arg" in
    --silent | -S)
      silentFix="true"
      ;;
    --fix-errors | -F)
      fixErrors="true"
      ;;
    --verbose | -C)
      debugLevel="1"
      ;;
    --dry-run | -D)
      dryRun="1"
      ;;
    *)
      errorExitSynopsis "Unrecognised argument: " "$arg"
      ;;
    esac
  done
}

parseArguments $*
moveToScriptDirectory

isDebugEnabled() {
  if [ -z "$debugLevel" ]; then
    return 1
  fi
}

debugOutput() {
  local line
  local matchNumber='^[0-9]+$'
  if [[ $lineCount =~ $matchNumber ]]; then
    line="$headerFullPath:$lineCount:   DEBUG: $*"
  else
    line="$headerFullPath:   DEBUG: $*"
  fi

  if isDebugEnabled; then
    echo "$line"
  elif [ -z "$debugBuffer" ]; then
    debugBuffer="$line"
  else
    debugBuffer="$debugBuffer\n$line"
  fi
}

# Generates the values for the stated filename, the header-guard definition, and the namespace that
# should be used in the header.
generateFullPathBasedValues() {
  local headerDirectory
  headerDirectory=$(dirname "$headerFullPath")
  headerDirectory=${headerDirectory/.\/include\//}

  local headerBackupDirectory
  headerBackupDirectory="$backupRoot/$headerDirectory"

  local headerBaseName
  headerBaseName=$(basename "$headerFullPath")
  headerStatedFileName="$headerDirectory/$headerBaseName"
  headerBackupFile="$headerBackupDirectory/$headerBaseName"
  headerTempFile="$headerBackupDirectory/.$headerBaseName"

  headerNamespace=${headerDirectory//\//::}
  headerNamespace=${headerNamespace//-/::}

  local headerGuardHalfFabricate
  headerGuardHalfFabricate="${headerDirectory}_M_$headerBaseName"

  headerGuard=${headerGuardHalfFabricate//\//_}
  headerGuard=${headerGuard//\./_}
  headerGuard=${headerGuard//\-/_}
  headerGuard=$(echo "$headerGuard" | sed -r 's|([a-z0-9])([A-Z])|\1_\2|g')
  headerGuard=$(echo "$headerGuard" | tr '[:lower:]' '[:upper:]')
}

# Scans the header file for structure and presence of certain elements.
# The, rather strict, expectation of the structure is as follows:
#
# 1:   #ifndef HEADER_GUARD
# 2:   #define HEADER_GUARD
# 3:   /*
# 4:    * stated-file-name.h
#      ...
# 5:   namespace name::space::name {
#      ...
# 6:   } // namespace name::space::name
#      ... (no or only empty lines)
# 7:   #endif // HEADER_GUARD
#      ... (no or only empty lines)
scanFileBasedValues() {

  local fileCommentStart=
  local matchFileGuardCheck='^\s*#ifndef\s+([_A-Z0-9]+)\s*$'
  local matchFileGuardDefinition
  local matchFileGuardEnd
  local matchFileCommentStart='^/\*\s*$'
  local matchFileStatedFile='^\s+\*\s*([-_A-Za-z0-9/]+\.h)\s*$'
  local matchFileNamespaceEnter='^\s*namespace\s+([a-z0-9A-Z:]+)\s+\{\s*$'
  local matchFileNamespaceLeave='}\s+//\s+namespace\s+[a-z0-9A-Z:]+\s*$'
  local matchEmptyLine='^\s*(//|)\s*$'

  local debugBuffer=
  local line
  local lineCount="1"

  while IFS= read -r line; do
    if [ -z "$fileLineGuardCheck" ]; then
      if [[ $line =~ $matchFileGuardCheck ]]; then
        fileValueGuard="${BASH_REMATCH[1]}"
        fileLineGuardCheck="$line"
        matchFileGuardDefinition="^\\s*#define\\s+$fileValueGuard\\s*\$"
        matchFileGuardEnd="^#endif\\s*//\\s*$fileValueGuard\\s*\$"
        debugOutput "Found guard '$fileValueGuard' check in line: $fileLineGuardCheck"
      elif ! [[ $line =~ matchEmptyLine ]]; then
        echo "$headerFullPath: WARNING: Non-empty lines before header-guard check -- bailing out" >&2
        echo "$headerFullPath:$lineCount: $line" >&2
        return 1
      fi
    elif [ -z "$fileLineGuardDefinition" ]; then
      if [[ $line =~ $matchFileGuardDefinition ]]; then
        fileLineGuardDefinition="$line"
        debugOutput "Found guard '$fileValueGuard' definition in line: $fileLineGuardDefinition"
      elif ! [[ $line =~ matchEmptyLine ]]; then
        echo "$headerFullPath: WARNING: Non-empty lines between header-guard check and definition -- bailing out" >&2
        echo "$headerFullPath:$lineCount: $line" >&2
        return 1
      fi
    elif [ -z "$fileCommentStart" ]; then
      if [[ $line =~ $matchFileCommentStart ]]; then
        fileCommentStart="$line"
        debugOutput "Found file-comment start in line: $fileCommentStart"
      elif ! [[ $line =~ matchEmptyLine ]]; then
        echo "$headerFullPath: WARNING: Non-empty lines between header-guard and top-level comment -- bailing out" >&2
        echo "$headerFullPath:$lineCount: $line" >&2
        return 1
      fi
    elif [ -z "$fileLineStatedFile" ]; then
      if [[ $line =~ $matchFileStatedFile ]]; then
        fileValueStatedFile="${BASH_REMATCH[1]}"
        fileLineStatedFile="$line"
        debugOutput "Found stated-file '$fileValueStatedFile' in line: $fileLineStatedFile"
      else
        echo "$headerFullPath: WARNING: Expecting stated file-name on first line of comment -- bailing out" >&2
        echo "$headerFullPath:$lineCount: $line" >&2
        return 1
      fi
    elif [ -z "$fileLineNamespaceEnter" ]; then
      if [[ $line =~ $matchFileNamespaceEnter ]]; then
        fileValueNamespace="${BASH_REMATCH[1]}"
        fileLineNamespaceEnter="$line"
        matchFileNamespaceLeave="}\\s*//\\s*namespace\\s+${fileValueNamespace}\\s*\$"
        debugOutput "Found namespace '$fileValueNamespace', entered in line: $fileLineNamespaceEnter"
      elif [[ $line =~ $matchFileGuardEnd ]]; then
        fileLineGuardEnd="$line"
        debugOutput "Found guard '$fileValueGuard' end in line: $fileLineGuardEnd"
        debugOutput "Skipping detection of namespace now."
        fileLineNamespaceEnter="\n"
        fileLineNamespaceLeave="\n"
      fi
    elif [ -z "$fileLineNamespaceLeave" ]; then
      if [[ $line =~ $matchFileNamespaceLeave ]]; then
        fileLineNamespaceLeave="$line"
        debugOutput "Found namespace '$fileValueNamespace', left in line: $fileLineNamespaceLeave"
      fi
    elif [ -z "$fileLineGuardEnd" ]; then
      if [[ $line =~ $matchFileNamespaceLeave ]]; then
        fileLineNamespaceLeave="$line"
        debugOutput "Found namespace '$fileValueNamespace', left (again) in line: $fileLineNamespaceLeave"
      elif [[ $line =~ $matchFileGuardEnd ]]; then
        fileLineGuardEnd="$line"
        debugOutput "Found guard '$fileValueGuard' end in line: $fileLineGuardEnd"
      fi
    elif ! [[ $line =~ matchEmptyLine ]]; then
      echo "$headerFullPath: WARNING: Non-empty lines after header-guard end -- bailing out" >&2
      echo "$headerFullPath:$lineCount: $line" >&2
      return 1
    fi
    lineCount=$(($lineCount + 1))
  done

  if [ -z "$fileLineGuardEnd" ]; then
    echo "$headerFullPath: WARNING: not able to find all elements needed for check and refactoring -- bailing out" >&2
    if ! isDebugEnabled; then
      echo -e "$debugBuffer" >&2
    fi
    return 1
  fi
}

setupBackupAndIntermediateFiles() {
  local tempDirectory
  tempDirectory=$(dirname "$headerBackupFile")

  if [ ! -d "${tempDirectory}" ]; then
    if ! mkdir -p "${tempDirectory}"; then
      echo "$headerFullPath: ERROR: Could not create directory for backup and intermediate results: ${tempDirectory}" >&2
      exit 1
    fi
  fi
  if ! cp "$headerFullPath" "$headerBackupFile"; then
    echo "$headerFullPath: ERROR: Could not make backup copy: $headerBackupFile" >&2
    exit 1
  fi
  if [ -f "$headerTempFile" ]; then
    if ! rm "$headerTempFile"; then
      echo "$headerFullPath: ERROR: Could not remove stale intermediate file: $headerTempFile" >&2
      exit 1
    fi
  fi
}

fixProblems() {
  if ! setupBackupAndIntermediateFiles ; then
    return 1
  fi


  local line
  while IFS= read -r line; do

    if [ "$line" == "$fileLineGuardCheck" ]; then
      echo "#ifndef $headerGuard" >> "$headerTempFile"
    elif [ "$line" == "$fileLineGuardDefinition" ]; then
      echo "#define $headerGuard" >> "$headerTempFile"
    elif [ "$line" == "$fileLineStatedFile" ]; then
      echo " * $headerStatedFileName" >> "$headerTempFile"
    elif [ "$line" == "$fileLineNamespaceEnter" ]; then
      echo "namespace $headerNamespace {" >> "$headerTempFile"
    elif [ "$line" == "$fileLineNamespaceLeave" ]; then
      echo "} // namespace $headerNamespace" >> "$headerTempFile"
    elif [ "$line" == "$fileLineGuardEnd" ]; then
      echo "#endif // $headerGuard" >> "$headerTempFile"
    else
      echo "$line" >> "$headerTempFile"
    fi
  done

}

checkSanity() {
  local headerFullPath
  headerFullPath="$1"
  local debugBuffer=

  local headerNamespace
  local headerStatedFileName
  local headerGuard
  local headerBackupFile
  local headerTempFile

  generateFullPathBasedValues

  if [ -n "$debugLevel" ]; then
    debugOutput "VALUES deduced from full header path"
    debugOutput "  headerNamespace      = $headerNamespace"
    debugOutput "  headerStatedFileName = $headerStatedFileName"
    debugOutput "  headerGuard          = $headerGuard"
    debugOutput "  headerBackupFile     = $headerBackupFile"
    debugOutput "  headerTempFile       = $headerTempFile"
  fi
  local fileLineGuardCheck=
  local fileLineGuardDefinition=
  local fileLineGuardEnd=
  local fileValueGuard=
  local fileLineStatedFile=
  local fileValueStatedFile=
  local fileLineNamespaceEnter=
  local fileLineNamespaceLeave=
  local fileValueNamespace=

  if scanFileBasedValues <"$headerFullPath"; then
    if [ -n "$debugLevel" ]; then
      debugOutput "VALUES read from actual header file"
      debugOutput "  fileLineGuardCheck = $fileLineGuardCheck"
      debugOutput "  fileLineGuardDefinition = $fileLineGuardDefinition"
      debugOutput "  fileLineGuardEnd = $fileLineGuardEnd"
      debugOutput "  fileValueGuard = $fileValueGuard"
      debugOutput "  fileLineStatedFile = $fileLineStatedFile"
      debugOutput "  fileValueStatedFile = $fileValueStatedFile"
      debugOutput "  fileLineNamespaceEnter = $fileLineNamespaceEnter"
      debugOutput "  fileLineNamespaceLeave = $fileLineNamespaceLeave"
      debugOutput "  fileValueNamespace = $fileValueNamespace"
    fi
    local foundProblems=

    if [ "$headerGuard" != "$fileValueGuard" ]; then
      foundProblems="${foundProblems}G"
      if [ -z "$silentFix" ]; then
        echo "$headerFullPath: PROBLEM: Should use header-guard '$headerGuard' instead of '$fileValueGuard'"
      fi
    else
      fileLineGuardCheck="\n"
      fileLineGuardDefinition="\n"
      fileLineGuardEnd="\n"
    fi

    if [ "$headerStatedFileName" != "$fileValueStatedFile" ]; then
      foundProblems="${foundProblems}F"
      if [ -z "$silentFix" ]; then
        echo "$headerFullPath: PROBLEM: Should state name as '$headerStatedFileName' instead of '$fileValueStatedFile'"
      fi
    else
      fileLineStatedFile="\n"
    fi

    if [ -n "$fileValueNamespace" ] && [ "$headerNamespace" != "$fileValueNamespace" ]; then
      foundProblems="${foundProblems}N"
      if [ -z "$silentFix" ]; then
        echo "$headerFullPath: PROBLEM: Should define namespace as '$headerNamespace' instead of '$fileValueNamespace'"
      fi
    else
      fileLineNamespaceEnter="\n"
      fileLineNamespaceLeave="\n"
    fi

    if [ -n "$foundProblems" ] && [ -n "$fixErrors" ]; then
      debugOutput "Fixing..."

      if fixProblems <"$headerFullPath"; then
        if [ -n "$dryRun" ]; then
          echo "$headerFullPath: would copy back processed from $headerTempFile"
        else
          cp "$headerTempFile" "$headerFullPath"
        fi
      fi
    fi

  fi
  # Demonstration on how to read from a function output:
  #  read -r headerFullPath headerBaseName headerDirectory headerNamespace headerStatedFileName headerGuard < <(generateHeaderInfo "$1")
}

#for name in `find .| grep -Ei './include/.*\.h' | sort` ; do  generateHeaderInfo "$name" ; done
for name in $(find . | grep -Ei './include/.*\.h' | sort); do checkSanity "$name"; done
