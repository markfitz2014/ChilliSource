package com.chillisource.core;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

import com.chillisource.core.InterfaceIDType;
import com.chillisource.core.LocalNotificationReceiver;
import com.chillisource.core.LocalNotificationService;

public class LocalNotificationNativeInterface extends INativeInterface
{
	private static Intent msCreateNotificationIntent;
	private static AlarmManager msAlarmManager;
	
	private final static String kstrIntentToTriggerMapName = "IntentIDsToTriggerTime";
	private final static String kstrIntentToNotificationMapName = "IntentIDsToNotificationIDs";
	private final static String kstrAppOpenedFromNotification = "AppOpenedFromNotification";
	private final static String kstrArrayOfKeysName = "kstrArrayOfKeysName";
	private final static String kstrArrayOfValuesName = "kstrArrayOfValuesName";
	
	private final static long kqwNotificationTimeLimit = 500 * 1000; //in milliseconds
	
	private  LocalNotificationService mLocalNotificationService;
	native public static void ApplicationDidReceiveLocalNotification(String[] astrKeys, String[] astrValues);

	//--------------------------------------------------------------
	/// Static Member Data
	//--------------------------------------------------------------
	public static InterfaceIDType InterfaceID = new InterfaceIDType("CLocalNotificationNativeInterface");
	//--------------------------------------------------------------
	/// Constructor
	//--------------------------------------------------------------
	public LocalNotificationNativeInterface()
	{
		super();
		
		msCreateNotificationIntent = new Intent(CSApplication.get().getActivityContext(), LocalNotificationReceiver.class);
		msCreateNotificationIntent.setAction("com.chillisource.nativeInterface.ALARM_NOTIFICATION_INTENT");
		
		msAlarmManager = (AlarmManager)CSApplication.get().getActivityContext().getSystemService(Context.ALARM_SERVICE);
	}
	//--------------------------------------------------------------
	/// Is A
	///
	/// @return whether or not this implements the given interface.
	//--------------------------------------------------------------
	@Override
	public boolean IsA(InterfaceIDType inInterfaceType) 
	{
		return (inInterfaceType.Equals(InterfaceID));
	}
	//-------------------------------------------------------------------
	/// Schedule Notification
	/// 
	/// method accessible from native for allowing System notifications
	/// to be scheduled for specified time.
	///
	/// @param the trigger time in milliseconds
	/// @param notification text
	/// @param notification ID
	/// @param string array of keys for params (used to create param map)
	/// @param string array of values for params (used to create param map) 
	//-------------------------------------------------------------------
	public void ScheduleNotification(long inqwTriggerTime, String instrBody, int indwNotificationID, int inePriority,String[] inastrKey, String[] inastrValue)
	{
		FreeOutOfDateIntentIDs();		
		
		//Pass any data to the intent so that it can be used when the Alarm is triggered
		for(int udwParam = 0; udwParam < inastrKey.length; ++udwParam)
		{
			msCreateNotificationIntent.putExtra(inastrKey[udwParam], inastrValue[udwParam]);
		}			
		msCreateNotificationIntent.putExtra("TriggerTime", inqwTriggerTime);
		msCreateNotificationIntent.putExtra("NotificationBody", instrBody);
		msCreateNotificationIntent.putExtra("NotificationID", indwNotificationID);
		msCreateNotificationIntent.putExtra("Priority", inePriority);	
		
		//Get the next unique id for the notification so that it will not overwrite any of the previously scheduled notifications.
		int dwUniqueIntentID = GetNextIntentID();
		
		//Map unique intent id to time it will be triggered, this will allow us to remove already fired intents later
		SharedPreferencesNativeInterface.SetLong(kstrIntentToTriggerMapName, Integer.toString(dwUniqueIntentID), inqwTriggerTime);
		//Map unique intent id to notifications id - this will allow uys to cancel individual notification ids later
		SharedPreferencesNativeInterface.SetInt(kstrIntentToNotificationMapName, Integer.toString(dwUniqueIntentID), indwNotificationID);
				
		//Send this along with the intent so that it can be removed on the other end
		msCreateNotificationIntent.putExtra("IntentID", dwUniqueIntentID);

		PendingIntent sPendingIntent = PendingIntent.getBroadcast(CSApplication.get().getActivityContext(), dwUniqueIntentID , msCreateNotificationIntent	, PendingIntent.FLAG_UPDATE_CURRENT);
	
        //set alarm to time of trigger
		msAlarmManager.set(AlarmManager.RTC_WAKEUP, inqwTriggerTime, sPendingIntent);
		
	}
	
	//-------------------------------------------------------------------
	/// Cancel By ID
	/// 
	/// method accessable from native for cancelling previously
	/// scheduled notifications by id
	///
	/// @param the id of notification to be cancelled
	//-------------------------------------------------------------------
	public void CancelByID(int inID)
	{
		
		SharedPreferences sSharedPreferences = CSApplication.get().getActivityContext().getSharedPreferences(kstrIntentToNotificationMapName, 0);
		Map<String, ?> sIntentIDMap = sSharedPreferences.getAll();
		
		List<Integer> aIntentIDsToBeRemoved = new ArrayList<Integer>();
		
		for(Map.Entry<String,?> entry : sIntentIDMap.entrySet())
		{
			if((Integer)entry.getValue() == inID)
			{
				aIntentIDsToBeRemoved.add(Integer.parseInt(entry.getKey()));
			}
		}
		for(int dwID = 0; dwID < aIntentIDsToBeRemoved.size(); ++dwID)
		{
			PendingIntent sPendingIntent = PendingIntent.getBroadcast(CSApplication.get().getActivityContext(), aIntentIDsToBeRemoved.get(dwID) , msCreateNotificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);
			msAlarmManager.cancel(sPendingIntent);
			SharedPreferencesNativeInterface.RemoveKey(kstrIntentToTriggerMapName, Integer.toString(aIntentIDsToBeRemoved.get(dwID)));
			SharedPreferencesNativeInterface.RemoveKey(kstrIntentToNotificationMapName, Integer.toString(aIntentIDsToBeRemoved.get(dwID)));
		}
        
	}
	
	//-------------------------------------------------------------------
	/// Cancel All
	/// 
	/// Method accessable from native for cancelling all previously
	/// scheduled notifications
	///
	//-------------------------------------------------------------------
	public void CancelAll()
	{
		SharedPreferences sSharedPreferences = CSApplication.get().getActivityContext().getSharedPreferences(kstrIntentToTriggerMapName, 0);
		Map<String, ?> sPendingIntentIDMap = sSharedPreferences.getAll();
		
		List<Integer> aIntentIDsToBeRemoved = new ArrayList<Integer>();
		
		for(Map.Entry<String,?> entry : sPendingIntentIDMap.entrySet())
		{
			aIntentIDsToBeRemoved.add(Integer.parseInt(entry.getKey()));
		}
		for(int dwID = 0; dwID < aIntentIDsToBeRemoved.size(); ++dwID)
		{
			PendingIntent sPendingIntent = PendingIntent.getBroadcast(CSApplication.get().getActivityContext(), aIntentIDsToBeRemoved.get(dwID) , msCreateNotificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);
			msAlarmManager.cancel(sPendingIntent);
			SharedPreferencesNativeInterface.RemoveKey(kstrIntentToTriggerMapName, Integer.toString(aIntentIDsToBeRemoved.get(dwID)));
			SharedPreferencesNativeInterface.RemoveKey(kstrIntentToNotificationMapName, Integer.toString(aIntentIDsToBeRemoved.get(dwID)));
		}
	}
	
	//-------------------------------------------------------------------
	/// Get Next Intent ID
	/// 
	/// Determines the next free ID for intent that is passed to the 
	/// the alarm manager to use so that it will not overwrite 
	/// previously placed alarms
	///
	/// @returns Unique ID for notification
	//-------------------------------------------------------------------
	private int GetNextIntentID()
	{
		Boolean bFoundUniqueID = false;
		int dwCount = 0;
		while(!bFoundUniqueID)
		{
			//if no notification currently uses this ID
			if(SharedPreferencesNativeInterface.KeyExists(kstrIntentToTriggerMapName, Integer.toString(dwCount)))
			{
				dwCount++;
			}
			else
			{
				Log.d("MoFlow", "Found Unique Intent ID - " + dwCount);
				bFoundUniqueID = true;
			}
		}
		return dwCount;
	}
	//-------------------------------------------------------------------
	/// Free Out Of Date Intent IDs
	/// 
	/// Goes through all currently placed intentIDs and checks the time
	///	that they were fired. If they were over the defined time limit
	/// then they will be removed allowing them to be overwritten.
	/// This prevents preferences from getting too large
	///
	//-------------------------------------------------------------------
	private void FreeOutOfDateIntentIDs()
	{
		SharedPreferences sSharedPreferences = CSApplication.get().getActivityContext().getSharedPreferences(kstrIntentToTriggerMapName, 0);
		
		Map<String, ?> sIntentIDMap = sSharedPreferences.getAll();
		List<String> aKeysToRemove = new ArrayList<String>();
		
		long qwCurrentTime = System.currentTimeMillis();
		
		for(Map.Entry<String,?> entry : sIntentIDMap.entrySet())
		{
			String strCurrentKey = entry.getKey();
			long dwValue =((Number)entry.getValue()).longValue();
			
			if(qwCurrentTime > dwValue + kqwNotificationTimeLimit)
			{
				aKeysToRemove.add(strCurrentKey);
			}
		}		
		for(int udwKey = 0 ; udwKey < aKeysToRemove.size(); ++udwKey)
		{
			SharedPreferencesNativeInterface.RemoveKey(kstrIntentToTriggerMapName, aKeysToRemove.get(udwKey));	
			SharedPreferencesNativeInterface.RemoveKey(kstrIntentToNotificationMapName, aKeysToRemove.get(udwKey));	
		}
	}

}