<?php
// $Id$

/**
 * A (fake) pager plugin that wraps around the actual pager.
 *
 * @ingroup views_pager_plugins
 */
class views_php_plugin_pager extends views_php_plugin_wrapper  {

  /**
   * Perform any needed actions just prior to the query executing.
   */
  public function pre_execute($query) {
    $this->wrapped->pre_execute($query);

    foreach (array(/*'argument',*/ 'field', 'sort', 'filter', /*'relationship'*/) as $type) {
      foreach ($this->wrapped->view->$type as $id => $handler) {
        if (is_callable(array($handler, 'php_pre_execute'))) {
          $handler->php_pre_execute();
        }
      }
    }

    $this->wrapped->view->query->set_limit(0);
    $this->wrapped->view->query->set_offset(0);
  }

  /**
   * Perform any needed actions just after the query executing.
   */
  public function post_execute(&$result) {
    foreach (array(/*'argument',*/ 'field', 'sort', 'filter', /*'relationship'*/) as $type) {
      foreach ($this->wrapped->view->$type as $id => $handler) {
        if (is_callable(array($handler, 'php_post_execute'))) {
          $handler->php_post_execute();
        }
      }
    }

    $offset = $this->wrapped->view->get_current_page() * $this->wrapped->view->get_items_per_page() + $this->wrapped->view->get_offset();
    $this->wrapped->view->result = array_slice($this->wrapped->view->result, $offset, $this->wrapped->view->get_items_per_page());

    $this->wrapped->post_execute($result);
  }


  function execute_count_query(&$count_query) {
    $this->wrapped->execute_count_query($count_query);
  }
}
