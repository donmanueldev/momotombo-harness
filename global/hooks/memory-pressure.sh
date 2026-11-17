#!/bin/bash
# Memory probe shared by the memory hooks and the status line.
#   memory-pressure.sh            one line ("swap 93% · free 31%"); exit 1 under pressure, 0 otherwise
#   memory-pressure.sh --report   the line plus what uses the memory, by dev tool and project
# Pressure is free memory under HARNESS_MEMORY_FREE_PCT, or swap over HARNESS_MEMORY_SWAP_PCT of its
# total while free memory is under HARNESS_MEMORY_SWAP_FREE_PCT. macOS keeps swap full long after a
# squeeze ends, so full swap alone is no pressure.
# Never pass memory_pressure -l, -p or -S: those flags simulate pressure.
set -euo pipefail

SWAP_LIMIT="${HARNESS_MEMORY_SWAP_PCT:-80}"
FREE_LIMIT="${HARNESS_MEMORY_FREE_PCT:-15}"
SWAP_FREE_LIMIT="${HARNESS_MEMORY_SWAP_FREE_PCT:-30}"

# "total = 16384.00M  used = 15842.12M  free = 541.88M  (encrypted)"
swap=$(sysctl -n vm.swapusage 2>/dev/null | awk '
    function mb(v) { return (v ~ /G$/) ? v * 1024 : v + 0 }
    { total = mb($3); used = mb($6) }
    END { if (total > 0) printf "%d", used * 100 / total; else print 0 }') || swap=""
free=$(memory_pressure 2>/dev/null | sed -n 's/^System-wide memory free percentage: \([0-9]*\)%$/\1/p') || free=""
[ -n "$swap" ] || [ -n "$free" ] || exit 0

line="swap ${swap:-?}% · free ${free:-?}%"
pressure=false
[ -n "$swap" ] && [ "$swap" -gt "$SWAP_LIMIT" ] && { [ -z "$free" ] || [ "$free" -lt "$SWAP_FREE_LIMIT" ]; } && pressure=true
[ -n "$free" ] && [ "$free" -lt "$FREE_LIMIT" ] && pressure=true

if [ "${1:-}" != --report ]; then
    echo "$line"
    $pressure && exit 1
    exit 0
fi

if $pressure; then
    echo "Memory is under pressure: $line (the warning starts at free under $FREE_LIMIT%, or swap over $SWAP_LIMIT% with free under $SWAP_FREE_LIMIT%)."
else
    echo "Memory: $line."
fi
# RSS counts shared pages once per process, so the sums are an upper bound.
ps -Ao rss=,command= | awk '
    # Names come from paths anyone can create, and they end up in the context of the agent.
    function safe(s) { gsub(/[^A-Za-z0-9._@ -]/, "", s); return substr(s, 1, 40) }
    {
        rss = $1; cmd = $0; sub(/^ *[0-9]+ +/, "", cmd)
        argv0 = cmd; sub(/ .*/, "", argv0); name = argv0; sub(/.*\//, "", name)
        dev = ""
        if (cmd ~ /CoreSimulator|Simulator\.app|launchd_sim/) dev = "iOS Simulator"
        else if (cmd ~ /playwright|Chromium|chrome-headless|headless_shell/) dev = "Playwright and Chromium"
        else if (name == "claude" || cmd ~ /\/[Cc]laude\.app\/|\/claude( |$)/) dev = "Claude sessions"
        else if (cmd ~ /Xcode\.app|xcodebuild|XCBBuildService|SourceKitService|swift-frontend/) dev = "Xcode and xcodebuild"
        else if (cmd ~ /GradleDaemon|KotlinCompileDaemon/ || name == "java") dev = "Gradle and java daemons"
        else if (cmd ~ /Docker\.app|com\.docker|Virtualization\.VirtualMachine|colima|OrbStack/ || name == "docker") dev = "Docker"
        else if (name ~ /^php/ || cmd ~ / artisan( |$)/) dev = "php and artisan"
        else if (name == "rg") dev = "rg"
        else if (name == "node" || name == "bun" || name == "deno") {
            # The project is the folder that holds the node_modules the command runs from.
            i = index(cmd, "/node_modules/")
            if (i > 0) { project = substr(cmd, 1, i - 1); sub(/.*\//, "", project); dev = "node: " safe(project) }
            else dev = "node: no project"
        }
        if (dev != "") { mem["dev\t" dev] += rss; count["dev\t" dev]++; next }
        app = cmd
        if (match(app, /\/[^\/]+\.app\//)) app = substr(app, RSTART + 1, RLENGTH - 6)
        else app = name
        app = safe(app)
        mem["app\t" app] += rss; count["app\t" app]++
    }
    END {
        for (k in mem) printf "%s\t%d\t%d\n", k, mem[k], count[k]
    }' | sort -t$'\t' -k3,3nr | awk -F'\t' '
    function size(kb) { return (kb >= 1048576) ? sprintf("%.1f GB", kb / 1048576) : sprintf("%d MB", kb / 1024) }
    $1 == "dev" && dev < 10 { d[++dev] = sprintf("  %-28s %8s  (%d process%s)", $2, size($3), $4, $4 == 1 ? "" : "es") }
    $1 == "app" && app < 5 { a[++app] = sprintf("  %-28s %8s", $2, size($3)) }
    END {
        print "Dev tools using the most memory (RSS):"
        if (!dev) print "  none"
        for (i = 1; i <= dev; i++) print d[i]
        print "Other apps (information only):"
        for (i = 1; i <= app; i++) print a[i]
    }'
echo "Offer to stop what isn't needed; stop nothing without the user's yes."
$pressure && exit 1
exit 0
