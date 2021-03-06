<?php

/**
 * @file
 * Extends EntityAPIControllerInterface with classes for Activities.
 */

/**
 * CRM Activity Entity Class.
 */
class CRMCoreActivityEntity extends Entity {
  protected function defaultLabel() {
    return $this->title;
  }

  protected function defaultUri() {
    return array(
      'path' => 'crm-core/activity/' . $this->identifier(),
      'options' => array(
        'absolute' => TRUE,
      ),
    );
  }
}

/**
 * Controller class for CRM Activity.
 */
class CRMCoreActivityController extends EntityAPIController {
  public function create(array $values = array()) {
    $query = new EntityFieldQuery();
    $results = $query
      ->entityCondition('entity_type', 'crm_core_activity_type')
      ->propertyCondition('type', $values['type'])
      ->execute();
    if (!empty($results)) {
      $type_id = 0;
      foreach (array_keys($results['crm_core_activity_type']) as $id) {
        $type_id = $id;
      }
      $values += array(
        'activity_type_id' => $type_id,
      );
    }
    $values += array(
      'title' => '',
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
      'uid' => $GLOBALS['user']->uid,
    );

    return parent::create($values);
  }
}

/**
 * Provides a class for activities.
 *
 * Use a separate class so we can specify some defaults modules may alter.
 */
class CRMActivityType extends Entity {
  public $type;
  public $label;
  public $weight = 0;

  public function __construct($values = array()) {
    parent::__construct($values, 'crm_core_activity_type');
  }

  function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}

/**
 * Controller class for CRM Activity Type.
 */
class CRMCoreActivityTypeController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    $values += array(
      'type' => '',
      'label' => '',
      'description' => '',
      'is_new' => TRUE,
    );

    return parent::create($values);
  }

  /**
   * Implements EntityAPIControllerInterface.
   *
   * @param array $ids
   *   List of IDs.
   * @param object $transaction
   *   Optionally a DatabaseTransaction object to use. Allows overrides to pass
   *   in their transaction object.
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    // Delete all instances of the given type.
    $query = new EntityFieldQuery();
    $results = $query
      ->entityCondition('entity_type', 'crm_core_activity')
      ->entityCondition('bundle', $ids, 'IN')
      ->execute();

    if (!empty($results)) {
      $activities_to_remove = array_keys($results['crm_core_activity']);
      crm_core_activity_delete_multiple($activities_to_remove);
      watchdog('crm_core_activity', 'Delete !count activities due to deletion of activity type.', array('!count' => count($activities_to_remove)), WATCHDOG_INFO);
    }

    // Finally delete the type itself.
    return parent::delete($ids, $transaction);
  }
}
