
import json
import os

file_path = '/Users/nikola/predict/analyses.json'

def fix_json():
    try:
        if not os.path.exists(file_path):
            print("File not found.")
            return

        with open(file_path, 'r') as f:
            data = json.load(f)

        if 'analyses' not in data:
            print("Invalid JSON structure: 'analyses' key missing.")
            return

        original_count = len(data['analyses'])
        valid_analyses = []

        for analysis in data['analyses']:
            is_valid = True
            
            # Check indicators for nulls
            if 'indicators' in analysis:
                indicators = analysis['indicators']
                # Check critical double fields
                for key in ['roc_10', 'roc_20', 'roc_5', 'adx', 'rsi']:
                     if key in indicators and indicators[key] is None:
                         is_valid = False
                         print(f"Removing analysis {analysis.get('id')} ({analysis.get('ticker')}): {key} is null")
                         break
            
            if not is_valid:
                continue

            # Check state_history for nulls
            if 'state_history' in analysis:
                new_history = []
                for state in analysis['state_history']:
                    if state.get('x') is None or state.get('y') is None or state.get('z') is None:
                        # Skip this state point, but keep analysis if it has other data?
                        # Actually, if state history is corrupted, maybe just clear it or remove analysis?
                        # Let's just remove the bad state points.
                        pass 
                    else:
                        new_history.append(state)
                analysis['state_history'] = new_history

            valid_analyses.append(analysis)

        data['analyses'] = valid_analyses
        new_count = len(valid_analyses)

        with open(file_path, 'w') as f:
            json.dump(data, f, indent=2)

        print(f"Fixed {file_path}. Removed {original_count - new_count} invalid analyses.")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    fix_json()
