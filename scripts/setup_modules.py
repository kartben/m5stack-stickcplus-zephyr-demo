import os
import sys
import urllib.request
import subprocess

import yaml
from InquirerPy import inquirer

def get_zephyr_revision():
    try:
        with open("west.yml", "r") as f:
            content = f.read()

        try:
            west_config = yaml.safe_load(content)
            projects = west_config.get("manifest", {}).get("projects", [])
            for project in projects:
                if project.get("name") == "zephyr":
                    return project.get("revision", "main")
        except Exception:
            # Fallback to regex if YAML fails (e.g. jinja tags)
            import re
            # Look for revision: <something>
            # This is a simple heuristic.
            match = re.search(r"revision:\s*(.+)", content)
            if match:
                rev = match.group(1).strip()
                # If it's a jinja tag, default to main
                if "{{" in rev:
                    return "main"
                return rev

    except Exception as e:
        print(f"Error reading west.yml: {e}")
        sys.exit(1)
    return "main"

def fetch_upstream_manifest(revision):
    url = f"https://raw.githubusercontent.com/zephyrproject-rtos/zephyr/{revision}/west.yml"
    print(f"Fetching upstream west.yml from {url}...")
    try:
        with urllib.request.urlopen(url) as response:
            return yaml.safe_load(response.read())
    except Exception as e:
        print(f"Error fetching upstream west.yml: {e}")
        if revision != "main":
             print("Retrying with 'main'...")
             return fetch_upstream_manifest("main")
        return None

def get_modules_from_manifest(manifest):
    if not manifest:
        return []

    projects = manifest.get("manifest", {}).get("projects", [])
    # Filter out zephyr itself
    return [p["name"] for p in projects if p.get("name") != "zephyr"]

def select_modules(modules):
    if not modules:
        print("No modules found in upstream manifest.")
        return []


    print("\nInteractive Module Selection")

    try:
        selected_modules = inquirer.fuzzy(
            message="Select Zephyr modules to include:",
            choices=modules,
            multiselect=True,
            max_height="70%",
            instruction="Type to search, TAB/Space to select, ENTER to confirm",
        ).execute()
    except Exception as e:
        print(f"Error during selection: {e}")
        return []

    print(f"\nSelected {len(selected_modules)} modules.")
    return selected_modules

def update_west_yml(selected_modules):
    # We use simple string replacement for the allowlist to avoid reformatting the whole file with PyYAML
    # which might lose comments or formatting.
    if not selected_modules:
        print("No modules selected (or all selected). Removing name-allowlist to include everything.")

    try:
        with open("west.yml", "r") as f:
            lines = f.readlines()

        with open("west.yml", "w") as f:
            skip_allowlist = False
            for line in lines:
                if "name-allowlist:" in line:
                    if not selected_modules:
                        # If we want all modules, we skip writing the allowlist block
                        skip_allowlist = True
                        continue
                    else:
                        f.write(line)
                        # Write our new list
                        for module in selected_modules:
                            f.write(f"          - {module}\n")
                        skip_allowlist = True # Skip the old list
                        continue

                if skip_allowlist:
                    stripped = line.strip()
                    # If line starts with -, it's a list item.
                    # If it's empty or comment, we might keep it or skip it.
                    # If it unindents, we stop skipping.
                    # But indentation is hard to track line by line without state.
                    # The template has:
                    #           - cmsis_6
                    #           - ...
                    # Next line is usually end of file or next section.
                    # We assume allowlist is the last thing in the import block or followed by less indentation.
                    # Simple heuristic: if it starts with - and has same indentation, skip.
                    # Or just skip until we see something that doesn't look like a list item.
                    if stripped.startswith("-"):
                        continue
                    else:
                        skip_allowlist = False

                f.write(line)

    except Exception as e:
        print(f"Error updating west.yml: {e}")

def main():

    print("Setting up Zephyr modules...")
    revision = get_zephyr_revision()
    print(f"Detected Zephyr revision: {revision}")

    manifest = fetch_upstream_manifest(revision)

    if manifest:
        modules = get_modules_from_manifest(manifest)
        selected = select_modules(modules)
        update_west_yml(selected)

    # Self-destruct
    try:
        os.remove(__file__)
    except:
        pass

if __name__ == "__main__":
    main()
