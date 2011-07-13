/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package aprofplot;

import java.util.*;

/**
 *
 * @author bruno
 */
public abstract class RoutineInfo implements Comparable<RoutineInfo> {

    private String name, dem_name, full_dem_name;
    private String address;
    private String image;
    private double max_time;
    //private double max_ratio;
    //private double mean_ratio;
    private double total_time;
    private int total_calls;
    ArrayList<TimeEntry> time_entries;

    public RoutineInfo(String name, String address, String image) {
        this.name = this.dem_name = this.full_dem_name = name;
        this.address = address;
        this.image = image.substring(image.lastIndexOf('/') + 1);
        max_time = 0;
        //max_ratio = 0;
        //mean_ratio = 0;
        total_time = 0;
        total_calls = 0;
        time_entries = new ArrayList<TimeEntry>();
    }

    public void addTimeEntry(TimeEntry t) {
        int old_te_index = time_entries.indexOf(t);
        if (old_te_index >= 0) {
            t = t.mergeWith(time_entries.get(old_te_index));
            time_entries.remove(old_te_index);
        }
        time_entries.add(t);
        if (t.getTime() > max_time) max_time = t.getTime();
        //if (t.getRatio() > max_ratio) max_ratio = t.getRatio();
        //mean_ratio = ((mean_ratio * (time_entries.size() - 1)) + t.getRatio()) / time_entries.size();
        total_time += t.getTime() * t.getOccurrences();
        total_calls += t.getOccurrences();
    }

    public String getName() {
        if (Main.getRtnDisplayMode() == Main.DEMANGLED) return this.dem_name;
        else return this.full_dem_name;
    }

    public String getRealName() {
        return this.name;
    }

    public String getDemName() {
        return this.dem_name;
    }

    public String getFullDemName() {
        return this.full_dem_name;
    }

    public void setDemName(String dn) {
        this.dem_name = dn;
    }

    public void setFullDemName(String fdn) {
        this.full_dem_name = fdn;
    }

    public String getAddress() {
        return this.address;
    }

    public String getImage() {
        return this.image;
    }

    public double getMaxTime() {
        return max_time;
    }

//    public double getMaxRatio() {
//        return max_ratio;
//    }

    public double getMeanRatio() {
        double sum = 0;
        for (int i = 0; i < time_entries.size(); i++) sum += time_entries.get(i).getRatio();
        return sum / time_entries.size();
        //return mean_ratio;
    }

    public double getTotalTime() {
        return total_time;
    }

    public int getTotalCalls() {
        return this.total_calls;
    }

    public ArrayList<TimeEntry> getTimeEntries() {
        //return (ArrayList<TimeEntry>)time_entries.clone();
        return new ArrayList<TimeEntry>(this.time_entries);
    }

    @Override
    public boolean equals(Object o) {
        if (o != null && getClass().equals(o.getClass())) {
            RoutineInfo rtn_info = (RoutineInfo)o;
            return rtn_info.address.equals(address);
        }
        else return false;
    }

    @Override
    public int hashCode() {
        int hash = 5;
        hash = 83 * hash + (this.name != null ? this.name.hashCode() : 0);
        hash = 83 * hash + (this.address != null ? this.address.hashCode() : 0);
        hash = 83 * hash + (this.image != null ? this.image.hashCode() : 0);
        return hash;
    }

    public int compareTo(RoutineInfo ri) {
        if (total_time == ri.total_time) return 0;
        if (total_time > ri.total_time) return 1;
        return -1;
    }

    public void sortTimeEntriesByAccesses() {
        Collections.sort(time_entries, new Comparator<TimeEntry> () {
           public int compare(TimeEntry t1, TimeEntry t2) {
               if (t1.getAccesses() == t2.getAccesses()) return 0;
               if (t1.getAccesses() > t2.getAccesses()) return 1;
               return -1;
           }
        });
    }

    public void sortTimeEntriesByTime() {
        Collections.sort(time_entries, new Comparator<TimeEntry> () {
           public int compare(TimeEntry t1, TimeEntry t2) {
               if (t1.getTime() == t2.getTime()) return 0;
               if (t1.getTime() > t2.getTime()) return 1;
               return -1;
           }
        });
    }

    public void sortTimeEntriesByRatio() {
        Collections.sort(time_entries, new Comparator<TimeEntry> () {
           public int compare(TimeEntry t1, TimeEntry t2) {
               if (t1.getRatio() == t2.getRatio()) return 0;
               if (t1.getRatio() > t2.getRatio()) return 1;
               return -1;
           }
        });
    }

    public void sortTimeEntriesByRatio(int type) {
        final int t = type;
        Collections.sort(time_entries, new Comparator<TimeEntry> () {
           public int compare(TimeEntry t1, TimeEntry t2) {
               if (t1.getRatio(t) == t2.getRatio(t)) return 0;
               if (t1.getRatio(t) > t2.getRatio(t)) return 1;
               return -1;
           }
        });
    }

    public void sortTimeEntriesByOccurrences() {
        Collections.sort(time_entries, new Comparator<TimeEntry> () {
           public int compare(TimeEntry t1, TimeEntry t2) {
               if (t1.getOccurrences() == t2.getOccurrences()) return 0;
               if (t1.getOccurrences() > t2.getOccurrences()) return 1;
               return -1;
           }
        });
    }
}
